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

        const std::string NOP_EVENT = "nop";
        const std::string LOCATION_INITIAL = "initial";
        const std::string LOCATION_INVARIANT = "invariant";
        const std::string LOCATION_COMMITTED = "committed";
        const std::string LOCATION_URGENT = "urgent";
        const std::string EDGE_DO = "do";
        const std::string EDGE_PROVIDED = "provided";

        class outputter {
         private:
          std::ostream &out_;
         public:
          outputter () : out_ (std::cout)
          {}

          outputter (std::ostream &out) : out_ (out)
          {}

          virtual ~outputter () = default;

          inline void
          system (std::string id)
          {
            out_ << "system: " << id << std::endl;
          }

          inline void
          process (std::string id)
          {
            out_ << "process: " << id << std::endl;
          }

          inline void
          event (std::string id)
          {
            out_ << "event: " << id << std::endl;
          }

          inline void
          clock (std::string id, int size = 1)
          {
            out_ << "clock: " << size << ":" << id << std::endl;
          }

          inline void
          intvar (int min, int max, int init, std::string id, int size = 1)
          {
            assert (min <= max);
            assert (min <= init && init <= max);

            out_ << "int:" << size << ":" << min << ":" << max << ":" << init
                 << ":" << id
                 << std::endl;
          }

          template<typename V>
          void
          attributes (basic_attributes_t<V> attr)
          {
            out_ << "{";
            bool first = true;
            for (auto kv : attr)
              {
                if (first) first = false;
                else out_ << ":";
                out_ << kv.first << ":" << kv.second;
              }
            out_ << "}";
          }

          inline void
          location (std::string process, std::string id, attributes_t attr)
          {
            out_ << "location:" << process << ":" << id;
            attributes (attr);
            out_ << std::endl;
          }

          inline void
          edge (std::string process, std::string src, std::string tgt,
                std::string e, attributes_t attr)
          {
            out_ << "edge:" << process << ":" << src << ":" << tgt << ":" << e;
            attributes (attr);
            out_ << std::endl;
          }

         private:
          inline void
          sync_element (std::string P, std::string ev, bool marked)
          {
            out_ << ":" << P << "@" << ev;
            if (marked)
              out_ << "?";
          }

         public:
          inline void
          sync (std::map<std::string, std::pair<std::string, bool>> sv)
          {
            assert (sv.size() >= 2);

            out_ << "sync";
            for (auto ei = sv.begin (); ei != sv.end (); ei++)
              sync_element (ei->first, ei->second.first, ei->second.second);
            out_ << std::endl;
          }

          inline
          void comment_ ()
          {}

          template<typename T, typename... Args>
          void
          comment_ (T t, Args... args)
          {
            out_ << t;
            comment_ (args...);
          }

          template<typename ...Args>
          void
          comment (Args... args)
          {
            out_ << "# ";
            comment_ (std::forward<Args> (args)...);
          }

          template<typename ...Args>
          void
          commentln (Args... args)
          {
            comment (std::forward<Args> (args)...);
            out_ << std::endl;
          }

          template<typename... Args>
          void
          commentml (Args... args)
          {
            std::stringstream strs;
            comment_ (strs, std::forward<Args> (args)...);
            std::string c;
            while (std::getline (strs, c, '\n'))
              {
                out_ << "# " << c << std::endl;
              }
          }

          std::ostream &stream ()
          {
            return out_;
          }
        };
    }
}

#endif /* UPPAAL_TO_TCHEKER_UTOT_TCHECKER_HH */

