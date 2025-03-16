#include "argparse.h"

#include <getopt.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "SDL.h"
#include "cap32.h"
#include "keyboard.h"
#include "stringutils.h"
#include "log.h"
#include "video.h"
#include "glfuncs.h"  // For HAVE_GL

const struct option long_options[] =
{
   {"autocmd",  required_argument, nullptr, 'a'},
   {"cfg_file", required_argument, nullptr, 'c'},
   {"inject", required_argument, nullptr, 'i'},
   {"offset", required_argument, nullptr, 'o'},
   {"override", required_argument, nullptr, 'O'},
   {"sym_file", required_argument, nullptr, 's'},
   {"version",  no_argument, nullptr, 'V'},
   {"help",     no_argument, nullptr, 'h'},
   {"verbose",  no_argument, nullptr, 'v'},
   {nullptr, 0, nullptr, 0},
};

void usage(std::ostream &os, char *progPath, int errcode)
{
   std::string progname, dirname;

   stringutils::splitPath(progPath, dirname, progname);

   os << "Usage: " << progname << " [options] <slotfile(s)>\n";
   os << "\nSupported options are:\n";
   os << "   -a/--autocmd=<command>: execute command as soon as the emulator starts.\n";
   os << "   -c/--cfg_file=<file>:   use <file> as the emulator configuration file instead of the default.\n";
   os << "   -h/--help:              shows this help\n";
   os << "   -i/--inject=<file>:     inject a binary in memory after the CPC startup finishes\n";
   os << "   -o/--offset=<address>:  offset at which to inject the binary provided with -i (default: 0x6000)\n";
   os << "   -O/--override:          override an option from the config. Can be repeated. (example: -O system.model=3)\n";
   os << "   -s/--sym_file=<file>:   use <file> as a source of symbols and entry points for disassembling in developers' tools.\n";
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
    { "CAP32_PHAZER", cap32_keystroke(CAP32_PHAZER) },
    { "CAP32_MF2STOP", cap32_keystroke(CAP32_MF2STOP) },
    { "CAP32_RESET", cap32_keystroke(CAP32_RESET) },
    { "CAP32_SCRNSHOT", cap32_keystroke(CAP32_SCRNSHOT) },
    { "CAP32_SPEED", cap32_keystroke(CAP32_SPEED) },
    { "CAP32_TAPEPLAY", cap32_keystroke(CAP32_TAPEPLAY) },
    { "CAP32_DEBUG", cap32_keystroke(CAP32_DEBUG) },
    { "CAP32_WAITBREAK", cap32_keystroke(CAP32_WAITBREAK) },
    { "CAP32_DELAY", cap32_keystroke(CAP32_DELAY) },
    { "CAP32_PASTE", cap32_keystroke(CAP32_PASTE) },
    { "CAP32_DEVTOOLS", cap32_keystroke(CAP32_DEVTOOLS) },
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
      c = getopt_long (argc, argv, "a:c:hi:o:O:s:vV",
                       long_options, &option_index);
      // Logs before processing of the -v will not be visible.
      LOG_DEBUG("Next option: " << c << "(" << static_cast<char>(c) << ")");

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

         case 'i':
            args.binFile = optarg;
            break;

         case 'o':
            args.binOffset = std::stol(optarg, nullptr, 0);
            break;

         case 'O':
            {
              std::string opt(optarg);
              bool invalid = false;
              auto key_value_separator = opt.find('=');
              if (key_value_separator == std::string::npos) invalid = true;
              std::string key = opt.substr(0, key_value_separator);
              std::string value = opt.substr(key_value_separator+1);
              auto section_item_separator = key.find('.');
              if (section_item_separator == std::string::npos) invalid = true;
              std::string section = key.substr(0, section_item_separator);
              std::string item = key.substr(section_item_separator+1);
              if (invalid || section.empty() || item.empty()) {
                LOG_ERROR("Couldn't parse override: '" << opt << "'");
              } else {
                args.cfgOverrides[section][item] = value;
                LOG_INFO("Override configuration: " << section << "." << item << " = " << value);
              }
              break;
            }

         case 's':
            args.symFilePath = optarg;
            break;

         case 'v':
            log_verbose = true;
            break;

         case 'V':
            // Version
            std::cout << "Caprice32 " << VERSION_STRING;
#ifdef HASH
            std::cout << (std::string(HASH).empty()?"":"-"+std::string(HASH));
#endif
            std::cout << "\n";

            // APP_PATH
            std::cout << "APP_PATH: ";
            #ifdef APP_PATH
            std::cout << APP_PATH;
            #else
            std::cout << "Not provided";
            #endif
            std::cout << std::endl;

            // Flags
            std::cout << "Compiled with:"
#ifdef HAVE_GL
                      << " HAVE_GL"
#endif
#ifdef DEBUG
                      << " DEBUG"
#endif
                      << "\n";

            // Video plugins
            std::cout << "Number of video plugins available: "
                      << video_plugin_list.size() << std::endl;
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
   LOG_DEBUG("slot_list: " << stringutils::join(slot_list, ","))
}
