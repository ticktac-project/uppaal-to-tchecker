/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */
#include <cassert>
#include "utot-tchecker.hh"
#include "utot-translate.hh"

using namespace UTAP;
using namespace utot;

static void
s_compute_range_bounds (type_t tsize, int &min, int &max)
{
  while (tsize.getKind () == Constants::LABEL)
    tsize = tsize[0];
  assert (tsize.isRange ());
  assert (tsize[0].getKind () == Constants::INT ||
          tsize[0].getKind () == Constants::SCALAR);

  auto t = tsize.getRange ();
  min = eval_integer_constant (t.first);
  max = eval_integer_constant (t.second);
  if (min > max)
    tr_err ("empty range: ", tsize);
}

static void
s_output_integer_variable (std::ostream &out, std::string vname, type_t type,
                           expression_t init)
{
  int min, max, initval;
  Constants::kind_t kind = type.getKind ();

  while (kind == Constants::LABEL)
    {
      type = type[0];
      kind = type.getKind ();
    }

  assert (kind == Constants::BOOL || kind == Constants::RANGE ||
          kind == Constants::CONSTANT);
  if (kind == Constants::BOOL)
    {
      min = 0;
      max = 1;
    }
  else if (kind == Constants::RANGE)
    s_compute_range_bounds (type, min, max);

  if (init.empty ())
    {
      initval = (min <= 0 && 0 <= max) ? 0 : min;
      warn ("enforcing initial value of variable '", vname, "' to ",
            initval, ".\n");
    }
  else
    initval = eval_integer_constant (init);

  if (kind == Constants::CONSTANT)
    min = max = initval;
  tchecker::intvar (out, min, max, initval, vname);
}

static void
s_enumerate_array_elements_decl (std::ostream &out, std::string arrayname,
                                 type_t type, expression_t init,
                                 context_prefix_t ctx, std::deque<int> &S)
{
  Constants::kind_t kind = type.getKind ();

  if (kind == Constants::ARRAY)
    {
      int min, max;
      type_t subtype = type.getSub ();
      bool noinit = init.empty ();

      s_compute_range_bounds (type.getArraySize (), min, max);

      if (max >= init.getSize () && !noinit)
        tr_err ("invalid initializer size for array type: ", init, ".");

      for (int i = min; i <= max; i++)
        {
          S.push_front (i);
          s_enumerate_array_elements_decl (out, arrayname, subtype,
                                           noinit ? init : init[i],
                                           ctx, S);
          S.pop_front ();
        }
    }
  else
    {
      std::ostringstream oss;
      oss << arrayname;
      for (int s : S)
        oss << "_" << s;

      switch (kind)
        {
          case Constants::CHANNEL:
          case Constants::BROADCAST:
          case Constants::URGENT:
            tchecker::event (out, oss.str ());
          break;
          default:
            s_output_integer_variable (out, oss.str (), type, init);
          break;
        }
    }
}

void
utot::translate_declarations (std::ostream &out,
                              const UTAP::declarations_t &decl,
                              context_prefix_t ctx)
{
  auto vitr = decl.variables.begin ();
  if (ctx->isToplevel ())
    vitr++; // skip t(0) variable
  for (; vitr != decl.variables.end (); vitr++)
    {
      variable_t v = *vitr;
      std::string vname = v.uid.getName ();
      type_t type = v.uid.getType ();
      Constants::kind_t kind = type.getKind ();

      for (; kind == Constants::LABEL; kind = type.getKind ())
        type = type[0];

      tchecker::comment (out, v.toString ());

      switch (kind)
        {
          case Constants::CONSTANT:
          case Constants::BOOL:
          case Constants::RANGE:
            s_output_integer_variable (out, vname, type, v.expr);
          break;

          case Constants::CHANNEL:
          case Constants::BROADCAST:
            tchecker::event (out, vname);
          break;

          case Constants::ARRAY:
            {
              std::deque<int> S;
              s_enumerate_array_elements_decl (out, vname, type, v.expr, ctx, S);
            }
          break;

          default:
            {
              UTOT_TRACE ("%s\n", string_of (v.uid).c_str ());
              tr_err ("don't know what to do with this kind of variable: ",
                      v.uid.getName ());
            }
          break;
        }
    }
}

