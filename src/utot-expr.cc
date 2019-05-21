/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */
#include <cassert>
#include "utot.hh"
#include "utot-translate.hh"

#define EVAL_BINARY(k_, e_, op_)            \
  case Constants::k_: \
  return (eval_integer_constant (p, (e_)[0]) op_  \
          eval_integer_constant (p, (e_)[1]))

using namespace UTAP;
using namespace utot;

int
utot::eval_integer_constant (UTAP::instance_t *p, const UTAP::expression_t &e)
{
  assert (e.getType ().isInteger () || e.getType ().isBoolean () ||
          e.getType ().isScalar ());

  switch (e.getKind ())
    {
      EVAL_BINARY (PLUS, e, +);
      EVAL_BINARY (MINUS, e, -);
      EVAL_BINARY (MULT, e, *);
      EVAL_BINARY (DIV, e, /);
      EVAL_BINARY (MOD, e, %);
      EVAL_BINARY (BIT_AND, e, &);
      EVAL_BINARY (BIT_OR, e, |);
      EVAL_BINARY (BIT_XOR, e, ^);
      EVAL_BINARY (BIT_LSHIFT, e, <<);
      EVAL_BINARY (BIT_RSHIFT, e, >>);
      EVAL_BINARY (AND, e, &&);
      EVAL_BINARY (OR, e, ||);
      EVAL_BINARY (LT, e, <);
      EVAL_BINARY (LE, e, <=);
      EVAL_BINARY (EQ, e, ==);
      EVAL_BINARY (NEQ, e, !=);
      EVAL_BINARY (GE, e, >=);
      EVAL_BINARY (GT, e, >);

      case Constants::MIN :
        {
          int v1 = eval_integer_constant (p, e[0]);
          int v2 = eval_integer_constant (p, e[1]);
          return v1 < v2 ? v1 : v2;
        }

      case Constants::MAX :
        {
          int v1 = eval_integer_constant (p, e[0]);
          int v2 = eval_integer_constant (p, e[1]);
          return v1 < v2 ? v2 : v1;
        }

      case Constants::NOT :
        return !eval_integer_constant (p, e[0]);

      case Constants::UNARY_MINUS :
        return -eval_integer_constant (p, e[0]);

      case Constants::INLINEIF :
        if (eval_integer_constant (p, e[0]))
          return eval_integer_constant (p, e[1]);
        else
          return eval_integer_constant (p, e[2]);

      case Constants::IDENTIFIER :
        {
          symbol_t s = e.getSymbol ();
          type_t t = s.getType ();
          if (! t.isConstant ()) {
            tr_err ("symbol '", s.getName (), "' is not a constant.\n");
          }
          variable_t *v = static_cast<variable_t *> (s.getData ());

          if (v == nullptr || v->expr.empty ())
            {
              if (p != nullptr && p->mapping.find (s) != p->mapping.end ())
                {
                  return eval_integer_constant (p, p->mapping[s]);
                }
              else
                {
                  tr_err ("unknown constant symbol '", s, "'.\n");
                }
            }
          else
            {
              return eval_integer_constant (p, v->expr);
            }
        }
      case Constants::CONSTANT :
        return e.getValue ();

      case Constants::ARRAY :
        {
          expression_t a = e[0];
          int index = eval_integer_constant (p, e[1]);
          return eval_integer_constant (p, a[index]);
        }

      case Constants::LIST :
        {
          expression_t a = e[0];
          int index = eval_integer_constant (p, e[1]);
          return eval_integer_constant (p, a[index]);
        }

      default:
        tr_err ("don't known how to translate expression '", e, "'.");
    }

  return 0;
}

static void
s_binary_op (std::ostream &out, UTAP::instance_t *p, UTAP::expression_t &e, const char *cppop,
             context_prefix_t ctx)
{
  assert (e.getSize () == 2);
  out << "(";
  translate_expression (out, p, e[0], ctx);
  out << " " << cppop << " ";
  translate_expression (out, p, e[1], ctx);
  out << ")";
}

static void
s_unary_op (std::ostream &out, UTAP::instance_t *p, UTAP::expression_t &e,
            const char *cppop,
            context_prefix_t ctx)
{
  assert (e.getSize () == 1);
  out << "(";
  translate_expression (out, p, e[0], ctx);
  out << ")" << cppop;
}

static void
s_unary_op (std::ostream &out, UTAP::instance_t *p, const char *cppop,
            UTAP::expression_t &e,
            context_prefix_t ctx)
{
  assert (e.getSize () == 1);
  out << cppop << "(";
  translate_expression (out, p, e[0], ctx);
  out << ")";
}

static void
s_unsupported_operator (const char *op)
{
  tr_err ("operator '", op, "' is not supported.");
}

#define BINARY_OP(kind, op) \
  case Constants::kind: s_binary_op (out, p, e, op, ctx); break;

#define PRE_UNARY_OP(kind, op) \
  case Constants::kind: s_unary_op (out, p, op, e, ctx); break;

#define POST_UNARY_OP(kind, op) \
  case Constants::kind: s_unary_op (out, p, e, op, ctx); break;

#define UNSUPPORTED(kind, op) \
  case Constants::kind: s_unsupported_operator (op); break;

void
utot::translate_expression (std::ostream &out, UTAP::instance_t *p,
                            UTAP::expression_t &e, utot::context_prefix_t ctx)
{
  switch (e.getKind ())
    {
      PRE_UNARY_OP (NOT, "!");
      PRE_UNARY_OP (UNARY_MINUS, "-");

      BINARY_OP (PLUS, "+");
      BINARY_OP (MINUS, "-");
      BINARY_OP (MULT, "*");
      BINARY_OP (DIV, "/");
      BINARY_OP (MOD, "%");
      BINARY_OP (AND, "&&");

      case Constants::OR :
        {
          out << "!(!(";
          translate_expression (out, p, e[0], ctx);
          out << ") && !(";
          translate_expression (out, p, e[1], ctx);
          out << "))";
          break;
        }

      BINARY_OP (LT, "<");
      BINARY_OP (LE, "<=");
      BINARY_OP (EQ, "==");
      BINARY_OP (NEQ, "!=");
      BINARY_OP (GE, ">=");
      BINARY_OP (GT, ">");

      case Constants::INLINEIF :
        if (e.getKind () == Constants::IDENTIFIER && e.getType ().isBoolean ())
          {
            out << "((";
            translate_expression (out, p, e[0], ctx);
            out << ") * (";
            translate_expression (out, p, e[1], ctx);
            out << ") + ((1 - ";
            translate_expression (out, p, e[0], ctx);
            out << ") * (";
            translate_expression (out, p, e[2], ctx);
            out << ")))";
          }
        else
          tr_err ("if-then-else unsupported with complex condition '", e, "'.");
      break;

      case Constants::IDENTIFIER :
        {
          symbol_t s = e.getSymbol ();
          if (p && p->mapping.find (s) != p->mapping.end ())
            translate_expression (out, p, p->mapping[s], ctx);
          else if (e.getSymbol ().getFrame ().hasParent ())
            out << utot::add_prefix (ctx, e.getSymbol ().getName ());
          else
            out << e.getSymbol ().getName ();
        }
      break;

      case Constants::CONSTANT :
        out << e.getValue ();
      break;

      case Constants::ARRAY :
        {
          int minsz, maxsz;
          type_t basetype;
          expression_t initval;

          if (is_one_dim_int_array_variable (p, e[0], minsz, maxsz))
            {
              translate_expression (out, p, e[0], ctx);
              out << "[";
              translate_expression (out, p, e[1], ctx);
              if (minsz != 0)
                out << "-" << minsz;
              out << "]";
            }
          else
            {
              translate_expression (out, p, e[0], ctx);
              out << "_" << eval_integer_constant (p, e[1]);
            }
        }
      break;

      case Constants::SYNC :
        translate_event_expression (out, p, e, ctx);
      break;

      UNSUPPORTED (BIT_AND, "&");
      UNSUPPORTED (BIT_OR, "|");
      UNSUPPORTED (BIT_XOR, "^");
      UNSUPPORTED (BIT_LSHIFT, "<<");
      UNSUPPORTED (BIT_RSHIFT, ">>");

      UNSUPPORTED (ASSIGN, "=");
      UNSUPPORTED (ASSPLUS, "+");
      UNSUPPORTED (ASSMINUS, "-");
      UNSUPPORTED (ASSDIV, "/");
      UNSUPPORTED (ASSMOD, "%");
      UNSUPPORTED (ASSMULT, "*");
      UNSUPPORTED (ASSAND, "&");
      UNSUPPORTED (ASSOR, "|");
      UNSUPPORTED (ASSXOR, "^");
      UNSUPPORTED (ASSLSHIFT, "<<");
      UNSUPPORTED (ASSRSHIFT, ">>");

      UNSUPPORTED (COMMA, ",");
      UNSUPPORTED (DOT, ".");

      UNSUPPORTED (MIN, "min");
      UNSUPPORTED (MAX, "max");
      UNSUPPORTED (FUNCALL, "function call");

      UNSUPPORTED (POSTINCREMENT, "++");
      UNSUPPORTED (PREINCREMENT, "++");
      UNSUPPORTED (POSTDECREMENT, "--");
      UNSUPPORTED (PREDECREMENT, "--");

      UNSUPPORTED (LIST, "list");

      default:
        tr_err ("don't known how to translate expression '", e, "'.");
    }
}

std::string
utot::translate_expression (UTAP::instance_t *p, UTAP::expression_t &expr,
                            context_prefix_t ctx)
{
  std::ostringstream oss;

  translate_expression (oss, p, expr, ctx);
  return oss.str ();
}

static void
s_assign_binary_op (std::ostream &out, UTAP::instance_t *p,
                    UTAP::expression_t &var, const char *op,
                    UTAP::expression_t &arg, context_prefix_t ctx)
{
  translate_expression (out, p, var, ctx);
  out << "=";
  translate_expression (out, p, var, ctx);
  out << op;
  translate_expression (out, p, arg, ctx);
}

static void
s_not_an_assignment (UTAP::expression_t &e)
{
  tr_err (":", e.getPosition ().start, ":", e.getPosition ().end,
          " expression '", e, "' is not an assignment.");
}

#define ASSIGN_OP(kind, op) \
  case Constants::kind: \
  s_assign_binary_op (out, p, e[0], op, e[1],ctx); break;

#define ASSIGN_INCDEC(kind, op) \
  case Constants::kind: \
  s_assign_binary_op (out, p, e[0], op, one, ctx); \
  break;

#define NOT_AN_ASSIGNMENT(kind, op) \
  case Constants::kind: s_not_an_assignment (e); break;

void
utot::translate_assignment (std::ostream &out, UTAP::instance_t *p,
                            UTAP::expression_t &e, context_prefix_t ctx)
{
  static UTAP::expression_t one = expression_t::createConstant (1);

  assert (!e.empty ());

  switch (e.getKind ())
    {
      case Constants::ASSIGN:
        {
          translate_expression (out, p, e[0], ctx);
          out << "=";
          translate_expression (out, p, e[1], ctx);
        }
      break;

      ASSIGN_OP (ASSPLUS, "+");
      ASSIGN_OP (ASSMINUS, "-");
      ASSIGN_OP (ASSDIV, "/");
      ASSIGN_OP (ASSMOD, "%");
      ASSIGN_OP (ASSMULT, "*");

      ASSIGN_INCDEC (POSTINCREMENT, "+");
      ASSIGN_INCDEC (PREINCREMENT, "+");
      ASSIGN_INCDEC (POSTDECREMENT, "-");
      ASSIGN_INCDEC (PREDECREMENT, "-");

      case Constants::COMMA:
        {
          translate_assignment (out, p, e[0], ctx);
          out << ";";
          translate_assignment (out, p, e[1], ctx);
        }
      break;

      case Constants::CONSTANT:
        if (!eval_integer_constant (nullptr, e))
          s_not_an_assignment (e);
        else
          out << "nop";
      break;

      NOT_AN_ASSIGNMENT (NOT, "!");
      NOT_AN_ASSIGNMENT (UNARY_MINUS, "-");
      NOT_AN_ASSIGNMENT (PLUS, "+");
      NOT_AN_ASSIGNMENT (MINUS, "-");
      NOT_AN_ASSIGNMENT (MULT, "*");
      NOT_AN_ASSIGNMENT (DIV, "/");
      NOT_AN_ASSIGNMENT (MOD, "%");
      NOT_AN_ASSIGNMENT (AND, "&&");
      NOT_AN_ASSIGNMENT (OR, "||");
      NOT_AN_ASSIGNMENT (LT, "<");
      NOT_AN_ASSIGNMENT (LE, "<=");
      NOT_AN_ASSIGNMENT (EQ, "==");
      NOT_AN_ASSIGNMENT (NEQ, "!=");
      NOT_AN_ASSIGNMENT (GE, ">=");
      NOT_AN_ASSIGNMENT (GT, ">");
      NOT_AN_ASSIGNMENT (INLINEIF, "if-then-else");
      NOT_AN_ASSIGNMENT (IDENTIFIER, "ident");
      NOT_AN_ASSIGNMENT (ARRAY, "[]");
      NOT_AN_ASSIGNMENT (BIT_AND, "&");
      NOT_AN_ASSIGNMENT (BIT_OR, "|");
      NOT_AN_ASSIGNMENT (BIT_XOR, "^");
      NOT_AN_ASSIGNMENT (BIT_LSHIFT, "<<");
      NOT_AN_ASSIGNMENT (BIT_RSHIFT, ">>");
      NOT_AN_ASSIGNMENT (DOT, ".");
      NOT_AN_ASSIGNMENT (MIN, "min");
      NOT_AN_ASSIGNMENT (MAX, "max");
      NOT_AN_ASSIGNMENT (FUNCALL, "function call");
      NOT_AN_ASSIGNMENT (LIST, "list");

      UNSUPPORTED (ASSAND, "&");
      UNSUPPORTED (ASSOR, "|");
      UNSUPPORTED (ASSXOR, "^");
      UNSUPPORTED (ASSLSHIFT, "<<");
      UNSUPPORTED (ASSRSHIFT, ">>");

      default:
        tr_err ("don't known how to translate expression '", e, "'.");
    }
}

std::string
utot::translate_assignment (UTAP::instance_t *p, UTAP::expression_t &expr,
                            context_prefix_t ctx)
{
  std::ostringstream oss;

  translate_assignment (oss, p, expr, ctx);

  return oss.str ();

}

void
utot::translate_event_expression (std::ostream &out, UTAP::instance_t *p,
                                  UTAP::expression_t &e, context_prefix_t ctx)
{
  switch (e.getKind ())
    {
      case Constants::IDENTIFIER :
        {
          symbol_t s = e.getSymbol ();
          if (p && p->mapping.find (s) != p->mapping.end ())
            translate_event_expression (out, p, p->mapping[s], ctx);
          else if (e.getSymbol ().getFrame ().hasParent ())
            out << utot::add_prefix (ctx, e.getSymbol ().getName ());
          else
            out << e.getSymbol ().getName ();
        }
      break;

      case Constants::ARRAY :
        {
          translate_event_expression (out, p, e[0], ctx);
          out << "_" << eval_integer_constant (p, e[1]);
          //translate_expression (out, p, e[1], ctx);
        }
      break;

      case Constants::SYNC:
        translate_event_expression (out, p, e[0], ctx);
      break;

      default:
        tr_err ("don't known how to translate event expression '", e, "'.");
    }
}

extern bool
utot::is_one_dim_int_array_variable (UTAP::instance_t *p, UTAP::expression_t e,
                                     int &minsz, int &maxsz)
{
  type_t basetype;
  expression_t initval;

  if (! is_one_dim_int_array_type (p, e.getType (), minsz, maxsz, basetype))
    return false;
  variable_t *v = (variable_t *) e.getSymbol ().getData ();
  return are_all_equals_in_list (p, v->expr, initval);
}
