/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */
#include <cassert>
#include <sstream>
#include <functional>
#include "utot.hh"
#include "utot-contextprefix.hh"
#include "utot-translate.hh"
#include "utot-tchecker.hh"

using namespace UTAP;
using namespace utot;

using event_label_t = std::string;
using proc_label_t = std::string;
using event_set_t = std::set<event_label_t>;
using proc_set_t = std::set<proc_label_t>;

struct global_events_t {
    std::map<event_label_t, proc_set_t> emitters;
    std::map<event_label_t, proc_set_t> receivers;
    std::map<event_label_t, proc_set_t> csp;
    event_set_t labels;
    event_set_t brodcasts;
};

enum symbol_type_t {
    EVENT,
    VARIABLE,
    PROCESS,
    CLOCK,
    LOCATION
};

static const proc_label_t STUCK_PROCESS = "Stuck";
static const event_label_t NO_SYNC_EVENT = "nosync";

static void
s_translate_states (context_prefix_t ctx, instance_t p,
                    tchecker::outputter &out)
{
  std::string prefix = string_of (ctx);

  for (state_t s : p.templ->states)
    {
      std::ostringstream oss;
      tchecker::attributes_t attr;

      if (s.invariant.empty ())
        oss << "1";
      else
        translate_expression (oss, &p, s.invariant, ctx);

      attr[tchecker::LOCATION_INVARIANT] = oss.str ();

      if (s.uid.getType ().is (Constants::COMMITTED))
        attr[tchecker::LOCATION_COMMITTED] = "";

      if (s.uid.getType ().is (Constants::URGENT))
        attr[tchecker::LOCATION_URGENT] = "";

      if (s.uid == p.templ->init)
        attr[tchecker::LOCATION_INITIAL] = "";

      out.location (prefix, s.uid.getName (), attr);
    }
}

static std::string
s_emit_event (std::string uevent)
{
  return uevent + "_emit";
}

static std::string
s_recv_event (std::string uevent)
{
  return uevent + "_recv";
}

static std::string
s_csp_event (std::string uevent)
{
  return uevent;
}

static bool
s_is_broadcast (type_t type)
{
  if (type.isArray ())
    return s_is_broadcast (type[0]);
  return type.is (Constants::BROADCAST);
}

static bool
s_is_broadcast (expression_t event)
{
  return s_is_broadcast (event.getSymbol ().getType ());
}

static std::string
s_build_edge_label (expression_t event, context_prefix_t ctx,
                    event_set_t &local_events,
                    global_events_t &events, instance_t p,
                    tchecker::outputter &tckout)
{
  std::string ev;
  std::ostringstream oss;
  std::map<event_label_t, proc_set_t> *eventmap;
  event_label_t s;
  bool local = event.getSymbol ().getFrame ().hasParent ();

  msg<VL_INFO> ("build edge label for event ", event, "\n");

  if (local)
    {
      // If the channel has been declared in the template we prefix it with
      // the ID of the process.
      ev = translate_expression (&p, event, ctx);

      if (local_events.find (ev) == local_events.end ())
        {
          tckout.event (ev);
          local_events.insert (ev);
        }
      s = ev;
    }
  else
    {
      // Since the event is global we don't have to use the process prefix.
      s = translate_expression (&p, event, context_prefix_t ());
      ev = s;
    }

  switch (event.getSync ())
    {
      case Constants::synchronisation_t::SYNC_BANG:
        {
          ev = s_emit_event (ev);
          eventmap = &events.emitters;
        }
      break;
      case Constants::synchronisation_t::SYNC_QUE:
        {
          ev = s_recv_event (ev);
          eventmap = &events.receivers;
        }
      break;
      case Constants::synchronisation_t::SYNC_CSP:
        {
          ev = s_csp_event (ev);
          eventmap = &events.csp;
        }
      break;
    }

  if (events.labels.find (ev) == events.labels.end ())
    {
      tckout.event (ev);
      events.labels.insert (ev);

      if (s_is_broadcast (event))
        events.brodcasts.insert (s);
    }

  proc_label_t P = string_of (ctx);
  proc_set_t *set;
  auto itr = eventmap->find (s);

  if (itr == eventmap->end ())
    {
      eventmap->emplace (s, proc_set_t ());
      set = &eventmap->find (s)->second;
    }
  else
    {
      set = &itr->second;
    }
  set->insert (P);

  return ev;
}

static void
s_translate_edge (edge_t e, context_prefix_t ctx, event_set_t &local_events,
                  global_events_t &events, instance_t p,
                  tchecker::outputter &tckout)
{
  std::string process = string_of (ctx);

  if (e.select.getSize () > 0)
    {
      tckout.comment ("edge with selection:");
      for (int i = 0; i < e.select.getSize (); i++)
        {
          symbol_t s = e.select.getSymbol (i);
          tckout.stream () << " " << s.getName () << " = " << p.mapping[s];
        }
      tckout.stream () << std::endl;
    }

  std::string ev;
  expression_t event = e.sync;

  if (event.empty ())
    ev = tchecker::NOP_EVENT;
  else
    ev = s_build_edge_label (event, ctx, local_events, events, p, tckout);

  std::string src = e.src->uid.getName ();
  std::string tgt = e.dst->uid.getName ();

  assert (e.actname == "SKIP" || e.actname == "");

  tchecker::attributes_t attr;

  attr[tchecker::EDGE_PROVIDED] = translate_expression (&p, e.guard, ctx);
  attr[tchecker::EDGE_DO] = translate_assignment (&p, e.assign, ctx);

  tckout.edge (process, src, tgt, ev, attr);
}

static void
s_translate_edge_with_select (edge_t e, int selectIdx, context_prefix_t ctx,
                              event_set_t &local_events,
                              global_events_t &events,
                              instance_t p, tchecker::outputter &tckout)
{
  if (selectIdx == e.select.getSize ())
    s_translate_edge (e, ctx, local_events, events, p, tckout);
  else
    {
      symbol_t sel = e.select.getSymbol (selectIdx);
      expression_t oldval;

      if (p.mapping.find (sel) != p.mapping.end ())
        oldval = p.mapping[sel];

      type_t type = sel.getType ();
      if (!type.isRange ())
        tr_err ("type of symbol '", sel.getName (), "' can not be enumerated.");

      auto r = type.getRange ();
      int min = utot::eval_integer_constant (&p, r.first);
      int max = utot::eval_integer_constant (&p, r.second);

      for (int i = min; i <= max; i++)
        {
          p.mapping[sel] = expression_t::createConstant (i);
          s_translate_edge_with_select (e, selectIdx + 1, ctx, local_events,
                                        events, p, tckout);
        }
      if (!oldval.empty ())
        p.mapping[sel] = oldval;
    }
}

static void
s_translate_edges (context_prefix_t ctx,
                   event_set_t &local_events,
                   global_events_t &events,
                   instance_t p, tchecker::outputter &tckout)
{
  std::string process = string_of (ctx);

  for (edge_t e : p.templ->edges)
    s_translate_edge_with_select (e, 0, ctx, local_events, events, p, tckout);
}

static void
s_translate_process (context_prefix_t ctx, instance_t p,
                     std::map<symbol_t, expression_t> mapping,
                     global_events_t &events, tchecker::outputter &tckout)
{
  event_set_t local_events;

  assert(mapping.size () == p.unbound + p.arguments);
  if (p.unbound > 0)
    tckout.commentln ("instantiation as ", ctx);

  for (auto kv: mapping)
    p.mapping[kv.first] = kv.second;
  std::string prefix = string_of (ctx);
  tckout.process (prefix);

  translate_declarations (tckout, &p, ctx, *p.templ);
  s_translate_states (ctx, p, tckout);
  s_translate_edges (ctx, local_events, events, p, tckout);

  tckout.stream () << std::endl;
}

static std::string
s_assignment_to_string (symbol_t s, int value)
{
  std::ostringstream oss;
  oss << s.getName () << "_" << value;
  return oss.str ();
}

static void
s_translate_process_rec (context_prefix_t ctx, instance_t p, int nb_unbound,
                         std::map<symbol_t, expression_t> mapping,
                         global_events_t &events, tchecker::outputter &tckout)
{
  if (nb_unbound == 0)
    s_translate_process (ctx, p, mapping, events, tckout);
  else
    {
      symbol_t s = p.parameters[nb_unbound - 1];
      type_t type = s.getType ();
      if (!type.isRange ())
        tr_err ("type of symbol '", s.getName (), "' can not be enumerated.");

      auto r = type.getRange ();
      int min = utot::eval_integer_constant (&p, r.first);
      int max = utot::eval_integer_constant (&p, r.second);

      for (int i = min; i <= max; i++)
        {
          std::string prefix = s_assignment_to_string (s, i);
          ctx.push_back (prefix);
          mapping[s] = expression_t::createConstant (i);
          s_translate_process_rec (ctx, p, nb_unbound - 1, mapping, events,
                                   tckout);
          ctx.pop_back ();
        }
    }
}

static void
s_translate_process (instance_t p, global_events_t &events,
                     tchecker::outputter &tckout)
{
  tckout.commentln ("compilation of process ", p.uid.getName ());
  context_prefix_t ctx;

  ctx.push_back (p.uid.getName ());

  if (p.unbound > 0)
    {
      msg<VL_INFO> ("enumerating values of parameters for partially "
                    "instantiated process '", p.uid.getName (), "'.\n");
      std::map<symbol_t, expression_t> mapping (p.mapping);
      s_translate_process_rec (ctx, p, p.unbound, mapping, events, tckout);
    }
  else s_translate_process (ctx, p, p.mapping, events, tckout);
}

static void
s_generate_emit_recv (tchecker::outputter &tckout, event_label_t s,
                      proc_label_t PE, proc_label_t PR)
{
  assert (PE != PR);

  std::string emit = s_emit_event (s);
  std::string recv = s_recv_event (s);
  std::map<std::string, std::pair<std::string, bool>> vect;

  vect.emplace (PE, std::make_pair (emit, false));
  vect.emplace (PR, std::make_pair (recv, false));

  tckout.sync (vect);
}

static void
s_generate_emit_broadcast (tchecker::outputter &tckout, event_label_t s,
                           proc_label_t PE, proc_set_t receivers)
{
  std::string emit = s_emit_event (s);
  std::map<std::string, std::pair<std::string, bool>> vect;

  vect.emplace (PE, std::make_pair (emit, false));
  for (proc_label_t PR : receivers)
    {
      if (PE == PR)
        continue;

      std::string recv = s_recv_event (s);
      vect.emplace (PR, std::make_pair (recv, true));
    }

  if (vect.size () >= 2)
    tckout.sync (vect);
}

static void
s_generate_strong_sync (tchecker::outputter &tckout, event_label_t s,
                        proc_set_t processes)
{
  if (processes.size () <= 1)
    return;

  std::map<std::string, std::pair<std::string, bool>> vect;

  for (proc_label_t P : processes)
    {
      event_label_t e = s_csp_event (s);
      vect.emplace (P, std::make_pair (e, false));
    }
  if (vect.size () >= 2)
    tckout.sync (vect);
}

static void
s_generate_blocked_event (tchecker::outputter &tckout, event_label_t s,
                          proc_label_t P,
                          std::function<std::string (event_label_t)> trevent)
{
  std::string ev = trevent (s);
  std::map<std::string, std::pair<std::string, bool>> vect;

  vect.emplace (P, std::make_pair (ev, false));
  vect.emplace (STUCK_PROCESS, std::make_pair (NO_SYNC_EVENT, false));
  tckout.sync (vect);
}

static void
s_generate_sync_vectors (tchecker::outputter &tckout, global_events_t &events)
{
  for (auto kv : events.emitters)
    {
      const event_label_t &s = kv.first;
      proc_set_t &emitters = kv.second;
      auto ir = events.receivers.find (s);
      bool norecv = (ir == events.receivers.end ());

      if (events.brodcasts.find (s) != events.brodcasts.end ())
        {
          if (norecv)
            continue;
          proc_set_t &receivers = ir->second;
          for (proc_label_t PE : emitters)
            s_generate_emit_broadcast (tckout, s, PE, receivers);
        }
      else if (norecv)
        {
          for (proc_label_t PE : emitters)
            s_generate_blocked_event (tckout, s, PE, s_emit_event);
        }
      else
        {
          proc_set_t &receivers = ir->second;

          for (proc_label_t PE : emitters)
            for (proc_label_t PR : receivers)
              s_generate_emit_recv (tckout, s, PE, PR);
        }
      events.receivers.erase (s);
    }

  for (auto kv : events.receivers)
    {
      const event_label_t &s = kv.first;
      proc_set_t &receivers = kv.second;
      for (proc_label_t PR : receivers)
        s_generate_blocked_event (tckout, s, PR, s_recv_event);
    }

  for (auto kv : events.csp)
    {
      const event_label_t &s = kv.first;
      proc_set_t &procs = kv.second;
      s_generate_strong_sync (tckout, s, procs);
    }
}

static void
s_add_stuck_process (tchecker::outputter &tckout)
{
  tckout.process (STUCK_PROCESS);
  tckout.event (NO_SYNC_EVENT);
  tchecker::attributes_t attr;
  attr[tchecker::LOCATION_INITIAL] = "";

  tckout.location (STUCK_PROCESS, "sink", attr);
}

static void
s_display_proc_set (event_label_t e, proc_set_t &procs, std::string action)
{
  msg<VL_INFO> (e, " ", action, ":");
  if (verbose_level >= VL_INFO)
    {
      for (auto p : procs)
        print (std::cout, " ", p);
      print (std::cout, "\n");
    }
}

static void
s_display_table_of_events (global_events_t &events)
{
  msg<VL_INFO> ("Table of events:\n");
  for (auto kv : events.emitters)
    s_display_proc_set (kv.first, kv.second, "is emitted by");

  for (auto kv : events.receivers)
    s_display_proc_set (kv.first, kv.second, "is received by");

  for (auto kv : events.csp)
    s_display_proc_set (kv.first, kv.second, "synchronizes");

  if (!events.labels.empty ())
    {
      msg<VL_INFO> ("global events:\n");
      for (auto e : events.labels)
        msg<VL_INFO> ("\t", e, "\n");
    }

  if (!events.brodcasts.empty ())
    {
      msg<VL_INFO> ("broadcast events:\n");
      for (auto e : events.brodcasts)
        msg<VL_INFO> ("\t", e, "\n");
    }
}

bool
utot::translate_model (TimedAutomataSystem &tas, tchecker::outputter &tckout)
{
  bool result;

  msg<VL_PROGRESS> ("starting translation.\n");
  try
    {
      context_prefix_t toplevel;
      global_events_t events;

      tckout.system ("S");
      tckout.commentln ("global event for Uppaal unlabelled edges");
      tckout.event ("nop");
      s_add_stuck_process (tckout);

      utot::translate_declarations (tckout, nullptr, toplevel, tas.getGlobals ());

      for (instance_t p : tas.getProcesses ())
        s_translate_process (p, events, tckout);
      s_display_table_of_events (events);
      s_generate_sync_vectors (tckout, events);

      result = true;
    }
  catch (const translation_exception &e)
    {
      result = false;
    }
  msg<VL_PROGRESS> ("translation terminated.\n");

  return result;
}