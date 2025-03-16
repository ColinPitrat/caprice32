.TH cap32 6 "June 2017"
.SH NAME
cap32 - Caprice32 Amstrad CPC emulator

.SH SYNOPSIS
.B cap32
[\fIOPTION\fR]... [\fIFILE\fR]...

.SH DESCRIPTION
\fBCaprice32\fR is an Amstrad CPC emulator. It emulates the Amstrad CPC464, 664, 6128 and 6128+ home computers.

.PP
\fBMedia loading\fR
.RS
Upon invocation, the FILEs specified as arguments will be used to populate the various available CPC machine slots. The type of slot is determined by the file format, detected by its extension. Caprice32 supports the following file formats:
\fB.dsk\fR (disk image), \fB.ipf\fR (Interchangeable Preservation Format disk image), \fB.raw\fR (CT-RAW format), \fB.voc\fR or \fB.cdt\fR (tape image), \fB.cpr\fR (cartridge image), \fB.sna\fR (snapshot image), \fB.zip\fR (zip of any previously described file).
Specifying slot content on the command line is optional.
.PP
Caprice32 supports two disk drives: drive A and drive B. If two disk image files (.dsk or .ipf) are provided, the first one will be loaded in drive A and the second one in drive B.
.PP
The \fBcart_path\fR, \fBsnap_path\fR, \fBdsk_path\fR and \fBtape_path\fR entries in the configuration file are \fIonly\fR used to specify the directory to open when loading/saving files in the GUI. The full path to the various slot contents must be provided on the command line.
.PP
The \fBrom_path\fR entry in the configuration file is used by the emulator to find its \fIsystem\fR ROM and cartridges.
.RE

.PP
\fBConfiguration\fR
.RS
When launched, Caprice32 will look for a configuration file in several locations. If a configuration file was specified using the \fB\-\-cfg_file\fR command line switch, Caprice32 will try and use it. If no configuration file was specified, or the configuration file specified does not exist, Caprice32 will try and open, in this order:
.br
  - \fB$CWD/cap32.cfg\fR ($CWD being the directory where the cap32 executable resides)
.br
  - \fBcap32.cfg\fR file in the location pointed to by the XDG_CONFIG_HOME environment variable. If XDG_CONFIG_HOME is undefined, it will look at $HOME/.config/ as default XDG_CONFIG_HOME directory.
.br
  - \fB$HOME/.cap32.cfg\fR for compatibility.
.br
  - \fB/etc/cap32.cfg\fR.
.br
Caprice32 will use the first valid file it finds. If no configuration file is found, a default configuration will be used.
.PP
The configuration file contains various configuration parameters, some of which can be modified from the GUI.
When saving the configuration from the GUI, it will be written in the configuration file following the same order than previously described, except for the addition of the write permission condition.
.PP
Most of the configuration file entries are commented in the configuration file provided by default with Caprice32. However, when overwriting the configuration file from the emulator menu, all comments will be lost.
.RE

.PP
\fBControl\fR
.RS
Emulator functionalities are available through function keys (F1-F12).
As CPC also had function keys, those are emulated by keys from the numpad.
This makes sense since their disposition on the CPC keyboard was similar to the numpad.
If your computer doesn't have a numpad (e.g laptop that doesn't support it with Fn key) the only way to access the CPC F1-F10 keys is through the virtual keyboard.
The emulator function key mapping can also be redefined through the host keyboard mapping file.
.RE
.PP
.RS
The emulator function default key mapping is:
.RS
.br
\fR\fBF1\fR - Show GUI (and pause the emulator)
.br
\fR\fBF2\fR - Toggle fullscreen / windowed mode
.br
\fR\fBF3\fR - Take screenshot
.br
\fR\fBF4\fR - Press tape reader play key
.br
\fR\fBF5\fR - Reset the emulator
.br
\fR\fBF6\fR - Multiface II stop (advanced users only)
.br
\fR\fBF7\fR - Toggle joystick emulation
.br
\fR\fBF8\fR - Toggle FPS display
.br
\fR\fBF9\fR - Toggle speed limitation
.br
\fR\fBF10\fR - Exit the emulator
.br
\fR\fBF12\fR - Toggle debug mode
.br
\fR\fBShift+F1\fR - Show virtual keyboard
.br
\fR\fBShift+F3\fR - Take a machine snapshot
.RE
.RE

.PP
\fBJoystick support\fR
.RS
Joystick emulation can be turned on with the F7 key. When joystick emulation is on, real joysticks are disabled, and the keyboard arrows (directions) as well as X and Z (fire buttons) keys are remapped to emulate the joystick 1 of the CPC machine.
.PP
Caprice32 can also be controlled completely from the joystick. The configuration file \fBjoystick_menu_button\fR and \fBjoystick_vkeyboard_button\fR entries allow binding menu and virtual keyboard invocations to some of the (host) joystick button.
.RE

.PP
\fBKeyboard mapping\fR
.RS
Caprice32 supports CPC English, French and Spanish keyboard layouts. The CPC keyboard layout is defined in the configuration file by the \fBkeyboard\fR entry (0: English, 1: French, 2: Spanish).
The host keyboard mapping is defined in a specific file, allowing to remap any host key to any specific key. The mapping file to use is set by the \fBkbd_layout\fR configuration entry.
The mapping file must be put in the resources directory, defined by the configuration file \fBresources_path\fR entry.
If the mapping file is not defined or not found, Caprice32 will default to a standard US keyboard map.
.RE

\" Missing sections to add:
\" Multiface 2 invocation
\" Memory tool usage
\" Slot loading order
\" Etc.

.SH OPTIONS
.PP
.TP
\fB\-a\fR, \fB\-\-autocmd\fR=\fICOMMAND\fR
pass command to execute to the emulator. The option can be repeated to pass multiple commands. For example: cap32 -a 'print "Hello"' -a 'print "World"'
.TP
\fB\-c\fR, \fB\-\-cfg_file\fR=\fIFILE\fR
use FILE as the emulator configuration file.
.TP
\fB\-h\fR, \fB\-\-help\fR
display short help and exits
.TP
\fB\-i\fR, \fB\-\-inject\fR
inject a binary in memory after the CPC startup finishes
.TP
\fB\-o\fR, \fB\-\-offset\fR
offset at which to inject the binary provided with -i (default: 0x6000)
.TP
\fB\-O\fR, \fB\-\-override\fR
override an option from the config. Can be repeated. (example: -O system.model=3)
.TP
\fB\-s\fR, \fB\-\-sym_file\fR=\fIfile\fR
use <file> as a source of symbols and entry points for disassembling in developers' tools.
.TP
\fB\-V\fR, \fB\-\-version\fR
display Caprice32 version and exits
.TP
\fB\-v\fR, \fB\-\-verbose\fR
be talkative about what the emulator is doing (mostly for debug builds)

.SH EXAMPLES
.PP
cap32 ./disk/sorcery.dsk ./trail.dsk
.RS
Launches cap32, loads the content of ./disk/sorcery.dsk in the drive A slot, and ./trail.dsk in the drive B slot.
.SH BUGS
CPC6128+ emulation is incomplete: vectored & DMA interrupts, analog joysticks and 8 bit printer are not emulated.
.PP
See https://github.com/ColinPitrat/caprice32/issues for the list of reported bugs.

.SH AUTHOR
.PP
Caprice32 was originally written by Ulrich Doewich.
.PP
This manual page covers Colin Pitrat's fork of Caprice32.
.PP
The screen capture code uses driedfruit SDL_SavePNG code (https://github.com/driedfruit/SDL_SavePNG).

.SH FILES
$HOME/.cap32.cfg
.br
/etc/cap32.cfg

.SH SEE ALSO
https://github.com/ColinPitrat/caprice32
.br
https://github.com/driedfruit/SDL_SavePNG
