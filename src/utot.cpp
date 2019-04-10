#include <iostream>
#include <cassert>
#include <getopt.h>
#include <utap/utap.h>
#include "utot.hh"

#define VERBOSE_OPTION "V"
#define DEBUG_OPTION "d"
#define HELP_OPTION "h"
#define XTA_OPTION "t"
#define XML_OPTION "x"
#define VERSION_OPTION "v"

int utot_verbose_level = 0;
bool utot_debug = false;

static bool show_help = false;
static bool show_version = false;
static bool use_xta_parser = true;
static const char *input_filename = NULL;
static const char *output_filename = NULL;

static const char *OPTION_STRING = \
  VERBOSE_OPTION \
  DEBUG_OPTION \
  HELP_OPTION \
  XTA_OPTION  \
  XML_OPTION  \
  VERSION_OPTION;

static const struct option OPTIONS[] = {
    {"debug",   no_argument, NULL, *DEBUG_OPTION},
    {"xta",     no_argument, NULL, *XTA_OPTION},
    {"xml",     no_argument, NULL, *XML_OPTION},
    {"version", no_argument, NULL, *VERSION_OPTION},
    {"verbose", no_argument, NULL, *VERBOSE_OPTION},
    {"help",    no_argument, NULL, *HELP_OPTION},
    {NULL,      no_argument, NULL, 0}
};

static void
s_usage (const char *cmd, std::ostream &out)
{
  out << "usage: " << cmd
      << " [options] [uppaal-input-file] [tchecker-output-file]" << std::endl
      << "where options are: " << std::endl
      << "--debug, -" << DEBUG_OPTION
      << " \t enable debug traces" << std::endl

      << "--help, -" << HELP_OPTION
      << " \t display this help message." << std::endl

      << "--verbose, -" << VERBOSE_OPTION
      << " \t increase the level of verbosity" << std::endl

      << "--version, -" << VERSION_OPTION
      << " \t display version number" << std::endl

      << "--xml, -" << XML_OPTION
      << " \t enforce XML format as input format" << std::endl

      << "--xta, -" << XTA_OPTION
      << " \t enforce XTA as input format" << std::endl

      << "-- \t\t specify the end of options (if necessary)" << std::endl
      << std::endl
      << "If no input file is specified, the standard input is used."
      << std::endl
      << "If several 'xta' or 'xml' options are used the last one is used."
      << std::endl
      << std::endl;
}

static bool
s_parser_options (int argc, char **argv)
{
  int c;

  while ((c = getopt_long (argc, argv, OPTION_STRING, OPTIONS, NULL)) != -1)
    {
      switch (c)
        {
          case *DEBUG_OPTION :
            utot_debug = true;
          break;

          case *VERBOSE_OPTION:
            utot_verbose_level++;
          break;

          case *HELP_OPTION:
            show_help = true;
          break;

          case *XTA_OPTION:
            use_xta_parser = true;
          break;

          case *XML_OPTION:
            use_xta_parser = false;
          break;

          case *VERSION_OPTION:
            show_version = true;
          break;

          case ':':
            std::cerr << "missing argument to option '" << optopt << "'."
                      << std::endl;
          break;
          case '?':
          default:
            assert(c == '?');
          return false;
        }
    }
  argc -= optind;
  argv += optind;

  if (argc > 2)
    {
      std::cerr << "exceed number of arguments." << std::endl;
      return false;
    }

  if (argc == 1)
    {
      input_filename = argv[0];
    }
  if (argc == 2)
    {
      output_filename = argv[1];
    }

  return true;
}

int
main (int argc, char **argv)
{
  int result = EXIT_FAILURE;

  if (s_parser_options (argc, argv))
    {
      if (show_help)
        s_usage (argv[0], std::cout);
      else if (show_version)
        std::cout << "utot (uppaal-to-tchecker) "
                  << UTOT_MAJOR_VERSION << "."
                  << UTOT_MINOR_VERSION << "."
                  << UTOT_PATCH_VERSION << std::endl;
      else
        {
          UTOT_VERBOSE (0, "input (%s) : %s\n",
                        use_xta_parser ? "xta" : "xml",
                        input_filename ? input_filename : "stdin");
          UTOT_VERBOSE (0, "output : %s\n",
                        output_filename ? output_filename : "stdout");

          if (use_xta_parser)
            {

            }
          else
            {

            }
          result = EXIT_SUCCESS;
          UTAP::TimedAutomataSystem tas;
        }
    }

    else
    {
      s_usage (argv[0], std::cerr);
    }

  return result;
}