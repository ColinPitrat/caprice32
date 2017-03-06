#include <getopt.h>
#include <iostream>
#include <fstream>
#include "cap32.h"

const struct option long_options[] =
{
   {"version", no_argument, (int *)NULL, 'V'},
   {"help",    no_argument, (int *)NULL, 'h'},
   {(const char *)NULL, 0,  (int *)NULL, 0},
};


void usage(std::ostream &os, int errcode)
{
   os << "Usage: cap32 [options] <slotfile(s)>\n";
   os << "\nOptions can be:\n";
   os << "   -V/--version: outputs version and exit\n";
   os << "   -h/--help:    shows this help\n";
   os << "\nslotfiles is a list of files giving the content of the various CPC ports.\n";
   os << "Supported ports files format are .dsk (disk), .cdt or .voc (tape), .cpr (cartridge), .sna (snapshot), or a .zip (archive containing one or more of the supported ports files.)\n";
   os << "\nExample: cap32 sorcery.dsk\n";
   os << "\nPress F1 when the emulator is running to show the in-application option menu.\n";
   exit(errcode);
}

void parseArguments(int argc, char **argv, std::vector<std::string>& slot_list)
{
   int option_index = 0;
   int c;

   optind = 0; // To please test framework, when this function is called multiple times !
   while(1) {
      c = getopt_long (argc, argv, "hV",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
         break;

      switch (c)
      {
         case 'V':
            std::cout << VERSION_STRING << "\n";
            break;

         case 'h':
            usage(std::cout, 0);
            break;

         case '?':
         default:
            usage(std::cerr, 1);
            break;
       }
   }

   /* All remaining command line arguments will go to the slot content list */
   slot_list.assign(argv+optind, argv+argc);
}

