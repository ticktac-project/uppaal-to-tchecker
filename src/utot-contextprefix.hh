/*
 * This file is part of the TChecker Project.
 * 
 * See files AUTHORS and LICENSE for copyright details.
 */

#ifndef UPPAAL_TO_TCHEKER_UTOT_CONTEXTPREFIX_HH
# define UPPAAL_TO_TCHEKER_UTOT_CONTEXTPREFIX_HH

# include <set>
# include <sstream>
# include <utap/utap.h>

namespace utot
{
    typedef std::deque<std::string> context_prefix_t;

    /*
    class ContextPrefix {

     public:
      using context_symbols_t = std::set<UTAP::symbol_t>;

      static context_prefix_t create ()
      {
        return std::shared_ptr<ContextPrefix>(new ContextPrefix());
      }

      static context_prefix_t create (std::string prefix,
                                      context_prefix_t parent = nullptr)
      {
        return std::shared_ptr<ContextPrefix>(new ContextPrefix(prefix, parent));
      }

      static context_prefix_t create (context_symbols_t symbols,
                                      std::string prefix,
                                      context_prefix_t parent = nullptr)
      {
        return std::shared_ptr<ContextPrefix>(new ContextPrefix(symbols,
            prefix, parent));
      }

      virtual ~ContextPrefix () = default;

      bool isToplevel() const {
        return getParent () == nullptr;
      }

      context_prefix_t getParent() const {
        return parent_;
      }

      void output_prefix (std::ostream &out) const {
        if (parent_ != nullptr)
          {
            parent_->output_prefix (out);
            out << ".";
          }
        out << prefix_;
      }

      void output_prefix (std::ostream &out, const UTAP::symbol_t &s) const
      {
        if (symbols_.find (s) != symbols_.end ())
          out << prefix_;
        else if (parent_ != nullptr)
          parent_->output_prefix (out, s);
      }

     private:
      ContextPrefix () : parent_ (nullptr), symbols_ (), prefix_ ()
      {}

      ContextPrefix (std::string prefix, context_prefix_t parent)
          : parent_ (parent), symbols_ (), prefix_ (prefix)
      {}

      ContextPrefix (context_symbols_t symbols, std::string prefix,
                     context_prefix_t parent)
          : parent_ (parent), symbols_ (symbols), prefix_ (prefix)
      {}

      context_prefix_t parent_;
      context_symbols_t symbols_;
      std::string prefix_;
    };*/

  inline std::string add_prefix (utot::context_prefix_t p, std::string name);
}

inline std::ostream &operator << (std::ostream &out, utot::context_prefix_t p) {
  auto si = p.begin();
  out << (*si);
  for(si++; si != p.end(); si++)
    out << "_" << *si;
  return out;
}

inline std::string utot::add_prefix (utot::context_prefix_t p,
    std::string name)
{
  if (p.empty ())
    return name;
  std::ostringstream oss;
  oss << p << "_" << name;

  return oss.str ();
}

#endif /* UPPAAL_TO_TCHEKER_UTOT_CONTEXTPREFIX_HH */

