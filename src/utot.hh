/*
 * This file is part of the Tchecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_HH
# define UPPAAL_TO_TCHEKER_UTOT_HH

# include <cstdio>

# ifndef UTOT_MAJOR_VERSION
#  define UTOT_MAJOR_VERSION 'x'
# endif /* UTOT_MAJOR_VERSION */

# ifndef UTOT_MINOR_VERSION
#  define UTOT_MINOR_VERSION 'x'
# endif /* UTOT_MINOR_VERSION */

# ifndef UTOT_PATCH_VERSION
#  define UTOT_PATCH_VERSION 'x'
# endif /* UTOT_PATCH_VERSION */

extern bool utot_debug;
extern int utot_verbose_level;

# define UTOT_TRACE(fmt,...) \
do { \
  if(utot_debug) { \
      fprintf (stderr, "%s:%d: [%s] " fmt "\n", __FILE__, __LINE__, \
      __FUNCTION__, ##  __VA_ARGS__); \
  } \
} while (0)

# define UTOT_VERBOSE(level, fmt,...) \
do { \
  if(level < utot_verbose_level) { \
    fprintf(stdout, fmt, ## __VA_ARGS__); \
  } \
} while (0)


#endif /* UPPAAL_TO_TCHEKER_UTOT_HH */