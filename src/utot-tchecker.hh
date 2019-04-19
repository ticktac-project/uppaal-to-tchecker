/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_TCHECKER_HH
# define UPPAAL_TO_TCHEKER_UTOT_TCHECKER_HH

# include <cassert>
# include <iostream>
# include <sstream>
# include <string>
# include <algorithm>
# include <iterator>
# include <vector>
# include <map>
# include "utot.hh"

namespace utot
{
    namespace tchecker
    {
        template<typename V>
        using basic_attributes_t = std::map<std::string, V>;

        using attributes_t = basic_attributes_t<std::string>;

        const std::string INVARIANT = "invariant";
        const std::string COMMITTED = "committed";
        const std::string URGENT = "urgent";

        inline void
        system (std::ostream &out, std::string id)
        {
          out << "system: " << id << std::endl;
        }

        inline void
        process (std::ostream &out, std::string id)
        {
          out << "process: " << id << std::endl;
        }

        inline void
        event (std::ostream &out, std::string id)
        {
          out << "event: " << id << std::endl;
        }

        inline void
        clock (std::ostream &out, std::string id, int size = 1)
        {
          out << "clock: " << size << ":" << id << std::endl;
        }

        inline void
        intvar (std::ostream &out, int min, int max, int init, std::string id,
                int size = 1)
        {
          assert (min <= max);
          assert (min <= init && init <= max);

          out << "int:" << size << ":" << min << ":" << max << ":" << init
              << ":" << id
              << std::endl;
        }

        template<typename V>
        void
        attributes (std::ostream &out, basic_attributes_t<V> attr)
        {
          out << "{";
          bool first = true;
          for (auto kv : attr)
            {
              if (first) first = false;
              else out << ":";
              out << kv.first << ":" << kv.second;
            }
          out << "}";
        }

        inline void
        location (std::ostream &out, std::string process, std::string id,
                  attributes_t attr)
        {
          out << "location:" << process << ":" << id;
          attributes (out, attr);
          out << std::endl;
        }

        inline void
        edge (std::ostream &out, std::string process, std::string src,
              std::string tgt, std::string e, attributes_t attr)
        {
          out << "edge:" << process << ":" << src << ":" << tgt << ":" << e;
          attributes (out, attr);
          out << std::endl;
        }

        inline void comment_ (std::ostream &out) { }

        template<typename T, typename... Args>
        void
        comment_ (std::ostream &out, T t, Args... args)
        {
          out << t;
          comment_ (out, args...);
        }

        template<typename ...Args>
        void
        comment (std::ostream &out, Args... args)
        {
          out << "# ";
          comment_ (out,  std::forward<Args>(args)...);
          out << std::endl;
        }

        template <typename... Args> void
        commentml(std::ostream &out, Args... args)
        {
          std::stringstream strs;
          comment_ (strs, std::forward<Args>(args)...);
          std::string c;
          while (std::getline(strs, c, '\n')) {
              out << "# " << c << std::endl;
          }
        }
    }
}

#endif /* UPPAAL_TO_TCHEKER_UTOT_TCHECKER_HH */

