/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_HH
# define UPPAAL_TO_TCHEKER_UTOT_HH

# include <cstdio>
# include <string>
# include <sstream>
# include <iostream>

# define UTOT_TRACE(fmt, ...) \
do { \
  if(utot::debug) { \
      fprintf (stderr, "%s:%d: [%s] " fmt "\n", __FILE__, __LINE__, \
      __FUNCTION__, ##  __VA_ARGS__); \
  } \
} while (0)

namespace utot
{
    class exception : public std::exception {
    };

    extern bool debug;

    extern int verbose_level;

    const int VL_WARNING = 0;
    const int VL_PROGRESS = 1;
    const int VL_INFO = 2;

    template<typename T>
    void
    print (std::ostream &out, T t)
    {
      out << t;
    }

    template<typename T, typename ...Args>
    void
    print (std::ostream &out, T t, Args... args)
    {
      out << t;
      print (out, args...);
    }

    template<typename ...Args>
    void
    err_ex (const exception &e, Args... args)
    {
      std::cerr << "error: ";
      print (std::cerr, std::forward<Args> (args)...);
      std::cerr << std::endl;
      throw e;
    }

    template<typename... Args>
    void
    err (Args... args)
    {
      err_ex (exception (), std::forward<Args> (args)...);
    }

    template <int level>
    bool level_enabled() { return level < utot::verbose_level; }

    template<int level, typename... Args>
    void
    msg (Args... args)
    {
      if (! level_enabled<level>())
        return;

      if (level > 0)
        std::cout << std::string (level - 1, ' ');
      print (std::cout, std::forward<Args> (args)...);
      std::cout.flush ();
    }

    template<typename... Args>
    void
    warn (Args... args) {
      msg<VL_WARNING>("warning: ", std::forward<Args>(args)...);
    }

    template<typename T>
    std::string string_of (T t)
    {
      std::ostringstream oss;
      oss << t;
      return oss.str ();
    }

}

#endif /* UPPAAL_TO_TCHEKER_UTOT_HH */