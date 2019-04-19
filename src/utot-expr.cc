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
  return (eval_integer_constant ((e_)[0]) op_  \
                  eval_integer_constant ((e_)[1]))

using namespace UTAP;
using namespace utot;

int
utot::eval_integer_constant (const UTAP::expression_t &e)
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
          int v1 = eval_integer_constant (e[0]);
          int v2 = eval_integer_constant (e[1]);
          return v1 < v2 ? v1 : v2;
        }

      case Constants::MAX :
        {
          int v1 = eval_integer_constant (e[0]);
          int v2 = eval_integer_constant (e[1]);
          return v1 < v2 ? v2 : v1;
        }

      case Constants::NOT :
        return !eval_integer_constant (e[0]);

      case Constants::UNARY_MINUS :
        return -eval_integer_constant (e[0]);

      case Constants::INLINEIF :
        if (eval_integer_constant (e[0]))
          return eval_integer_constant (e[1]);
        else
          return eval_integer_constant (e[2]);

      case Constants::IDENTIFIER :
        {
          symbol_t s = e.getSymbol ();
          variable_t *v = static_cast<variable_t *> (s.getData ());
          assert (!v->expr.empty ());
          return eval_integer_constant (v->expr);
        }
      case Constants::CONSTANT :
        return e.getValue ();

      case Constants::ARRAY :
        {
          expression_t a = e[0];
          int index = eval_integer_constant (e[1]);
          return eval_integer_constant (a[index]);
        }

      case Constants::LIST :
        {
          expression_t a = e[0];
          int index = eval_integer_constant (e[1]);
          return eval_integer_constant (a[index]);
        }

      default:
        tr_err ("don't known how to translate expression '", e, "'.");
    }

  return 0;
}

static void
s_binary_op (std::ostream &out, const UTAP::expression_t &e, const char *cppop,
             context_prefix_t ctx)
{
  assert (e.getSize () == 2);
  translate_expression (out, e[0], ctx);
  out << " " << cppop << " ";
  translate_expression (out, e[1], ctx);
}

static void
s_unary_op (std::ostream &out, const UTAP::expression_t &e,
            const char *cppop, context_prefix_t ctx)
{
  assert (e.getSize () == 1);
  translate_expression (out, e[0], ctx);
  out << cppop;
}

static void
s_unary_op (std::ostream &out, const char *cppop,
            const UTAP::expression_t &e, context_prefix_t ctx)
{
  assert (e.getSize () == 1);
  out << cppop;
  translate_expression (out, e[0], ctx);
}

static void
s_unsupported_operator (const char *op)
{
  tr_err ("operator '", op, "' is not supported.");
}

#define BINARY_OP(kind, op) \
  case Constants::kind: s_binary_op (out, e, op, ctx); break;
#define UNARY_OP(kind, op) \
  case Constants::kind: s_unary_op (out, e, op, ctx); break;

#define UNSUPPORTED(kind, op) \
  case Constants::kind: s_unsupported_operator (op); break;

void
utot::translate_expression (std::ostream &out, const UTAP::expression_t &e,
                            context_prefix_t ctx)
{
  switch (e.getKind ())
    {
      UNARY_OP (NOT, "!");
      UNARY_OP (UNARY_MINUS, "-");

      BINARY_OP (PLUS, "+");
      BINARY_OP (MINUS, "-");
      BINARY_OP (MULT, "*");
      BINARY_OP (DIV, "/");
      BINARY_OP (MOD, "%");
      BINARY_OP (AND, "&&");

      case Constants::OR :
        {
          out << "!(!(";
          translate_expression (out, e[0], ctx);
          out << ") && !(";
          translate_expression (out, e[1], ctx);
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
            translate_expression (out, e[0], ctx);
            out << ") * (";
            translate_expression (out, e[1], ctx);
            out << ") + ((1 - ";
            translate_expression (out, e[0], ctx);
            out << ") * (";
            translate_expression (out, e[2], ctx);
            out << ")))";
          }
        else
          tr_err ("if-then-else unsupported with complex condition '", e, "'.");
      break;

      case Constants::IDENTIFIER :
        out << e.getSymbol ().getName ();
      break;

      case Constants::CONSTANT :
        out << e.getValue ();
      break;

      case Constants::ARRAY :
        {
          translate_expression (out, e[0], ctx);
          out << "[";
          translate_expression (out, e[1], ctx);
          out << "]";
        }
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

extern void
translate_assignment (std::ostream &out, const UTAP::expression_t &expr,
                      context_prefix_t ctx)
{

}
