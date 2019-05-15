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
s_compute_range_bounds (UTAP::instance_t *p, type_t tsize, int &min, int &max)
{
  while (tsize.getKind () == Constants::LABEL)
    tsize = tsize[0];
  assert (tsize.isRange ());
  assert (tsize[0].getKind () == Constants::INT ||
          tsize[0].getKind () == Constants::SCALAR);

  auto t = tsize.getRange ();
  min = eval_integer_constant (p, t.first);
  max = eval_integer_constant (p, t.second);
  if (min > max)
    tr_err ("empty range: ", tsize);
}

static void
s_output_integer_variable (tchecker::outputter &tckout, UTAP::instance_t *p,
                           std::string vname, type_t type,
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
    s_compute_range_bounds (p, type, min, max);

  if (init.empty ())
    {
      initval = (min <= 0 && 0 <= max) ? 0 : min;
      warn ("enforcing initial value of variable '", vname, "' to ",
            initval, ".\n");
    }
  else
    initval = eval_integer_constant (p, init);

  if (kind == Constants::CONSTANT)
    min = max = initval;

  if (!(min <= initval && initval <= max))
    tr_err ("initial value ", initval, " is out of the domain of variable ",
            vname);
  tckout.intvar (min, max, initval, vname);
}

static void
s_enumerate_array_elements_decl (tchecker::outputter &tckout,
                                 UTAP::instance_t *p,
                                 std::string arrayname,
                                 type_t type, expression_t init,
                                 std::deque<int> &S)
{
  Constants::kind_t kind = type.getKind ();

  if (kind == Constants::ARRAY)
    {
      int min, max;
      type_t subtype = type.getSub ();
      bool noinit = init.empty ();

      s_compute_range_bounds (p, type.getArraySize (), min, max);

      int size = init.getSize ();
      if ((max - min + 1) > size && !noinit)
        tr_err ("invalid initializer size for array type: ", init, ".");

      for (int i = min; i <= max; i++)
        {
          S.push_front (i);
          s_enumerate_array_elements_decl (tckout, p, arrayname, subtype,
                                           noinit ? init : init[i - min], S);
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
            //tchecker::event (out, oss.str ());
            tckout.commentln ("event to synchronize: ", oss.str ());
          break;

          case Constants::CLOCK:
            tckout.clock (oss.str (), 1);
          break;

          default:
            s_output_integer_variable (tckout, p, oss.str (), type, init);
          break;
        }
    }
}

bool
utot::is_one_dim_int_array_variable (UTAP::instance_t *p, UTAP::type_t t,
                                     int &minsz, int &maxsz,
                                     UTAP::type_t &basetype)
{
  assert (t.getKind () == Constants::ARRAY);
  s_compute_range_bounds (p, t.getArraySize (), minsz, maxsz);
  basetype = t[0];
  while (basetype.getKind () == Constants::LABEL)
    basetype = basetype[0];

  return (basetype.getKind () == Constants::BOOL ||
          basetype.getKind () == Constants::RANGE);
}

bool
utot::are_all_equals_in_list (UTAP::instance_t *p, UTAP::expression_t expr,
                              UTAP::expression_t &val)
{
  if (expr.empty ())
    return true;

  assert (expr.getKind () == Constants::LIST);

  int minsz, maxsz;
  s_compute_range_bounds (p, expr.getType ().getArraySize (), minsz, maxsz);
  int sz = maxsz - minsz + 1;
  if (expr.getSize () != sz)
    tr_err ("expression { ", expr, " } should have ", sz, " elements.");

  val = expr[0];
  for (int i = 1; i < sz; i++)
    {
      if (!val.equal (expr[i]))
        return false;
    }
  return true;
}

static void
s_declare_one_dim_array (tchecker::outputter &tckout, instance_t *p,
                         std::string vname, int size, type_t eltype,
                         expression_t init)
{
  int min, max;

  tckout.commentln ("one dim array.");
  if (eltype.isBoolean ())
    {
      min = 0;
      max = 1;
    }
  else
    {
      min = eval_integer_constant (p, eltype.getRange ().first);
      max = eval_integer_constant (p, eltype.getRange ().second);
    }

  int ival = (init.empty () ? min : eval_integer_constant (p, init));

  if (min <= ival && ival <= max)
    tckout.intvar (min, max, ival, vname, size);
  else
    tr_err ("initial value ", ival, " is out of range [", min, ",", max,
            "] for array '", vname, "'.");
}

void
utot::translate_declarations (tchecker::outputter &tckout, UTAP::instance_t *p,
                              context_prefix_t ctx, UTAP::declarations_t &decl)
{
  auto vitr = decl.variables.begin ();
  if (p == nullptr)
    vitr++; // skip t(0) variable
  for (; vitr != decl.variables.end (); vitr++)
    {
      variable_t v = *vitr;

      std::string vname = v.uid.getName ();

      type_t type = v.uid.getType ();
      Constants::kind_t kind = type.getKind ();

      for (; kind == Constants::LABEL; kind = type.getKind ())
        type = type[0];

      tckout.commentln (vname, ":", type.toString ());
      vname = add_prefix (ctx, vname);

      switch (kind)
        {
          case Constants::CONSTANT:
          case Constants::BOOL:
          case Constants::RANGE:
            s_output_integer_variable (tckout, p, vname, type, v.expr);
          break;

          case Constants::CHANNEL:
          case Constants::BROADCAST:
          case Constants::URGENT:
            //tchecker::event (out, vname);
            tckout.commentln ("global event to synchronize: ", vname);

          break;

          case Constants::CLOCK:
            tckout.clock (vname, 1);
          break;

          case Constants::ARRAY:
            {
              int minsz, maxsz;
              type_t basetype;
              UTAP::expression_t initval;

              if (is_one_dim_int_array_variable (p, type, minsz, maxsz, basetype)
                  && are_all_equals_in_list (p, v.expr, initval))
                {
                  s_declare_one_dim_array (tckout, p, vname, maxsz - minsz + 1,
                                           basetype, initval);
                }
              else
                {
                  std::deque<int> S;
                  s_enumerate_array_elements_decl (tckout, p, vname, type,
                                                   v.expr, S);
                }
            }
          break;

          default:
            {
              tr_err ("don't know what to do with this kind of variable: ",
                      v.uid.getName ());
            }
          break;
        }
    }
}

