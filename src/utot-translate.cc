/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */
#include <cassert>
#include <sstream>
#include "utot.hh"
#include "utot-contextprefix.hh"
#include "utot-translate.hh"
#include "utap/typechecker.h"
#include "utot-tchecker.hh"

using namespace UTAP;
using namespace utot;

enum symbol_type_t {
    EVENT,
    VARIABLE,
    PROCESS,
    CLOCK,
    LOCATION
};

static std::map<std::string, symbol_type_t> symbols;

static void
s_translate_process (context_prefix_t ctx, instance_t p,
                     std::map<symbol_t, expression_t> mapping,
                     std::ostream &out)
{
  assert(mapping.size () == p.unbound + p.arguments);
  if (p.unbound > 0)
    tchecker::comment (out, "instantiation as ", ctx);

  std::string prefix = string_of (ctx);
  tchecker::process (out, prefix);

  for (state_t s : p.templ->states)
    {
      std::ostringstream inv;
      tchecker::attributes_t attr;

      if (! s.invariant.empty ())
        translate_expression (inv, s.invariant, ctx);
      attr[tchecker::INVARIANT] = inv.str();

      if (s.uid.getType ().is (Constants::COMMITTED))
        attr[tchecker::COMMITTED] = "";

      if (s.uid.getType ().is (Constants::URGENT))
        attr[tchecker::URGENT] = "";

      tchecker::location (out, prefix, s.uid.getName (), attr);
    }

  out << std::endl;
}

static std::string
s_assignment_to_string (symbol_t s, int value)
{
  std::ostringstream oss;
  oss << s.getName () << "_" << value;
  return oss.str ();
}

static void
s_translate_process_rec (context_prefix_t ctx,
                         instance_t p, int nb_unbound,
                         std::map<symbol_t, expression_t> mapping,
                         std::ostream &out)
{
  if (nb_unbound == 0)
    s_translate_process (ctx, p, mapping, out);
  else
    {
      symbol_t s = p.parameters[nb_unbound - 1];
      type_t type = s.getType ();
      if (!type.isRange ())
        tr_err ("type of symbol '", s.getName (), "' can not be enumerated.");

      auto r = type.getRange ();
      int min = utot::eval_integer_constant (r.first);
      int max = utot::eval_integer_constant (r.second);

      for (int i = min; i <= max; i++)
        {
          std::string prefix = s_assignment_to_string (s, i);
          context_prefix_t lctx = ContextPrefix::create (prefix, ctx);
          mapping[s] = expression_t::createConstant (i);
          s_translate_process_rec (lctx, p, nb_unbound - 1, mapping, out);
        }
    }
}

static void
s_translate_process (instance_t p, std::ostream &out)
{
  tchecker::comment (out, "compilation of process ", p.uid.getName (), " ", 12);
  context_prefix_t ctx = ContextPrefix::create (p.uid.getName ());

  if (p.unbound > 0)
    {
      msg<VL_INFO> ("enumerating values of parameters for partially "
                    "instantiated process '", p.uid.getName (),"'.\n");
      std::map<symbol_t, expression_t> mapping (p.mapping);
      s_translate_process_rec (ctx, p, p.unbound, mapping, out);
    }
  else s_translate_process (ctx, p, p.mapping, out);
}

bool
utot::translate_model (TimedAutomataSystem &tas, std::ostream &out)
{
  bool result;

  TypeChecker tc (&tas);

  msg<VL_PROGRESS> ("starting translation.\n");
  try
    {
      context_prefix_t toplevel = ContextPrefix::create ();

      tchecker::system(out, "S");

      utot::translate_declarations (out, tas.getGlobals (), toplevel);

      for (instance_t p : tas.getProcesses ())
        s_translate_process (p, out);

      result = true;
    }
  catch (const std::exception &e)
    {
      result = true;
    }
  msg<VL_PROGRESS>("translation terminated.\n");

  return result;
}