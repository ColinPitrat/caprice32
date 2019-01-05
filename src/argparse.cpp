#include "argparse.h"

#include <getopt.h>
#include <iostream>
#include <fstream>
#include "SDL.h"
#include "cap32.h"
#include "keyboard.h"
#include "stringutils.h"
#include "log.h"

extern bool log_verbose;

const struct option long_options[] =
{
   {"autocmd",  required_argument, nullptr, 'a'},
   {"cfg_file", required_argument, nullptr, 'c'},
   {"version",  no_argument, nullptr, 'V'},
   {"help",     no_argument, nullptr, 'h'},
   {"verbose",  no_argument, nullptr, 'v'},
   {nullptr, 0, nullptr, 0},
};

CapriceArgs::CapriceArgs()
{
}

void usage(std::ostream &os, char *progPath, int errcode)
{
   std::string progname, dirname;

   stringutils::splitPath(progPath, dirname, progname);

   os << "Usage: " << progname << " [options] <slotfile(s)>\n";
   os << "\nSupported options are:\n";
   os << "   -a/--autocmd=<command>: execute command as soon as the emulator starts.\n";
   os << "   -c/--cfg_file=<file>:   use <file> as the emulator configuration file instead of the default.\n";
   os << "   -h/--help:              shows this help\n";
   os << "   -V/--version:           outputs version and exit\n";
   os << "   -v/--verbose:           be talkative\n";
   os << "\nslotfiles is an optional list of files giving the content of the various CPC ports.\n";
   os << "Ports files are identified by their extension. Supported formats are .dsk (disk), .cdt or .voc (tape), .cpr (cartridge), .sna (snapshot), or .zip (archive containing one or more of the supported ports files).\n";
   os << "\nExample: " << progname << " sorcery.dsk\n";
   os << "\nPress F1 when the emulator is running to show the in-application option menu.\n";
   os << "\nSee https://github.com/ColinPitrat/caprice32 or check the man page (man cap32) for more extensive information.\n";
   exit(errcode);
}

std::string cap32_keystroke(CAP32_KEYS key) {
  return std::string("\f") + char(key);
}

std::string cpc_keystroke(CPC_KEYS key) {
  return std::string("\a") + char(key);
}

std::string replaceCap32Keys(std::string command)
{
  static std::map<std::string, std::string> keyNames = {
    { "CAP32_EXIT", cap32_keystroke(CAP32_EXIT) },
    { "CAP32_FPS", cap32_keystroke(CAP32_FPS) },
    { "CAP32_FULLSCRN", cap32_keystroke(CAP32_FULLSCRN) },
    { "CAP32_GUI", cap32_keystroke(CAP32_GUI) },
    { "CAP32_VKBD", cap32_keystroke(CAP32_VKBD) },
    { "CAP32_JOY", cap32_keystroke(CAP32_JOY) },
    { "CAP32_MF2STOP", cap32_keystroke(CAP32_MF2STOP) },
    { "CAP32_RESET", cap32_keystroke(CAP32_RESET) },
    { "CAP32_SCRNSHOT", cap32_keystroke(CAP32_SCRNSHOT) },
    { "CAP32_SPEED", cap32_keystroke(CAP32_SPEED) },
    { "CAP32_TAPEPLAY", cap32_keystroke(CAP32_TAPEPLAY) },
    { "CAP32_DEBUG", cap32_keystroke(CAP32_DEBUG) },
    { "CAP32_WAITBREAK", cap32_keystroke(CAP32_WAITBREAK) },
    { "CAP32_DELAY", cap32_keystroke(CAP32_DELAY) },
    { "CPC_F1", cpc_keystroke(CPC_F1) },
    { "CPC_F2", cpc_keystroke(CPC_F2) },
  };
  for (const auto& elt : keyNames)
  {
    size_t pos;
    while ((pos = command.find(elt.first)) != std::string::npos)
    {
      command.replace(pos, elt.first.size(), elt.second);
      LOG_VERBOSE("Recognized keyword: " << elt.first);
    }
  }
  return command;
}

void parseArguments(int argc, char **argv, std::vector<std::string>& slot_list, CapriceArgs& args)
{
   int option_index = 0;
   int c;

   optind = 0; // To please test framework, when this function is called multiple times !
   while(true) {
      c = getopt_long (argc, argv, "a:c:hvV",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
         break;

      switch (c)
      {
         case 'a':
            LOG_VERBOSE("Append to autocmd: " << optarg);
            args.autocmd += replaceCap32Keys(optarg);
            args.autocmd += "\n";
            break;

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
            std::cout << "Caprice32 " << VERSION_STRING;
#ifdef HASH
            std::cout << (std::string(HASH).empty()?"":"-"+std::string(HASH));
#endif
            std::cout << "\n";
            std::cout << "Compiled with:"
#ifdef HAVE_GL
                      << " HAVE_GL"
#endif
#ifdef DEBUG
                      << " DEBUG"
#endif
#ifdef	WITH_IPF 
                      << " WITH_IPF"
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

