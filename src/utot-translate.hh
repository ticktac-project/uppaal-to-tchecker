/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_TRANSLATE_HH
# define UPPAAL_TO_TCHEKER_UTOT_TRANSLATE_HH

# include <utap/utap.h>

namespace utot
{
    extern bool
    translate_model(UTAP::TimedAutomataSystem &tas, std::ostream &out);
}

#endif /* UPPAAL_TO_TCHEKER_UTOT_TRANSLATE_HH */

