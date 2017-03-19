#include <getopt.h>
#include <iostream>
#include <fstream>
#include "cap32.h"
#include "argparse.h"
#include "stringutils.h"

extern bool log_verbose;
std::string commit_hash = "8cfdd63e9c4bfd7c1bad3b11cb586ea89149e4a0";

const struct option long_options[] =
{
   {"cfg_file", required_argument, nullptr, 'c'},
   {"version",  no_argument, nullptr, 'V'},
   {"help",     no_argument, nullptr, 'h'},
   {"verbose",  no_argument, nullptr, 'v'},
   {nullptr, 0, nullptr, 0},
};

CapriceArgs::CapriceArgs(void) :
   cfgFilePath("") // Unneeded as string are initialized empty. But kept here for clarity.
{
}

void usage(std::ostream &os, char *progPath, int errcode)
{
   std::string progname, dirname;

   stringutils::splitPath(progPath, dirname, progname);

   os << "Usage: " << progname << " [options] <slotfile(s)>\n";
   os << "\nSupported options are:\n";
   os << "   -c/--cfg_file=<file>:   use <file> as the emulator configuration file instead of the default.\n";
   os << "   -h/--help:              shows this help\n";
   os << "   -V/--version:           outputs version and exit\n";
   os << "   -v/--verbose:           be talkative\n";
   os << "\nslotfiles is an optional list of files giving the content of the various CPC ports.\n";
   os << "Ports files are identified by their extension. Supported formats are .dsk (disk), .cdt or .voc (tape), .cpr (cartridge), .sna (snapshot), or .zip (archive containing one or more of the supported ports files).\n";
   os << "\nExample: " << progname << " sorcery.dsk\n";
   os << "\nPress F1 when the emulator is running to show the in-application option menu.\n";
   os << "See https://github.com/ColinPitrat/caprice32 for more extensive information.\n";
   exit(errcode);
}

void parseArguments(int argc, char **argv, std::vector<std::string>& slot_list, CapriceArgs& args)
{
   int option_index = 0;
   int c;

   optind = 0; // To please test framework, when this function is called multiple times !
   while(1) {
      c = getopt_long (argc, argv, "c:hvV",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
         break;

      switch (c)
      {
         case 'c':
            args.cfgFilePath = optarg;
            break;

         case 'h':
            usage(std::cout, argv[0], 0);
            break;

         case 'v':
            log_verbose = true;
            break;

         case 'V':
            std::cout << "Caprice32 " << VERSION_STRING << (commit_hash.empty()?"\n":"-"+commit_hash+"\n");
            std::cout << "Compiled with:"
#ifdef HAVE_GL
                      << " HAVE_GL"
#endif
#ifdef DEBUG
                      << " DEBUG"
#endif
                      << "\n";
            exit(0);
            break;

         case '?':
         default:
            usage(std::cerr, argv[0], 1);
            break;
       }
   }

   /* All remaining command line arguments will go to the slot content list */
   slot_list.assign(argv+optind, argv+argc);
}

