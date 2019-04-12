/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_HH
# define UPPAAL_TO_TCHEKER_UTOT_HH

# include <cstdio>

# define UTOT_TRACE(fmt, ...) \
do { \
  if(utot::debug) { \
      fprintf (stderr, "%s:%d: [%s] " fmt "\n", __FILE__, __LINE__, \
      __FUNCTION__, ##  __VA_ARGS__); \
  } \
} while (0)

# define UTOT_VERBOSE(level, fmt, ...) \
do { \
  if(level < utot::verbose_level) { \
    fprintf(stdout, fmt, ## __VA_ARGS__); \
    fflush(stdout); \
  } \
} while (0)

namespace utot
{
    extern bool debug;

    extern int verbose_level;

}

#endif /* UPPAAL_TO_TCHEKER_UTOT_HH */