/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_TRANSLATE_HH
# define UPPAAL_TO_TCHEKER_UTOT_TRANSLATE_HH

# include <utap/utap.h>
# include "utot.hh"
# include "utot-contextprefix.hh"

namespace utot
{
    class translation_exception : public exception {
    };

    extern void
    translate_expression (std::ostream &out, const UTAP::expression_t &expr,
                          context_prefix_t ctx);

    extern void
    translate_assignment (std::ostream &out, const UTAP::expression_t &expr,
                          context_prefix_t ctx);

    extern void
    translate_declarations (std::ostream &out,
                            const UTAP::declarations_t &decl,
                            context_prefix_t ctx);

    extern int
    eval_integer_constant (const UTAP::expression_t &expr);

    extern bool
    translate_model (UTAP::TimedAutomataSystem &tas, std::ostream &out);

    template<typename... Args> void
    tr_err (Args... args) {
      utot::err_ex (translation_exception(), std::forward<Args>(args) ...);
    }
}

#endif /* UPPAAL_TO_TCHEKER_UTOT_TRANSLATE_HH */

