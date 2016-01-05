# use "make RELEASE=TRUE" to create release executable

CC	= g++

GFLAGS	= -mwindows -Wall -Wstrict-prototypes

ifdef RELEASE
CFLAGS	= $(GFLAGS) -O2 -funroll-loops -ffast-math -fomit-frame-pointer -fno-strength-reduce -finline-functions -s
else
CFLAGS	= $(GFLAGS) -gstabs+
endif

IPATHS	= -I/mingw/SDL/include/SDL -L/mingw/SDL/lib

cap32: cap32.cpp crtc.o fdc.o glfuncs.o psg.o tape.o video.o z80.o cap32.h z80.h
	$(CC) $(CFLAGS) $(IPATHS) -o cap32.exe cap32.cpp crtc.o fdc.o glfuncs.o psg.o tape.o video.o z80.o -lmingw32 -lSDLmain -lSDL -lz

crtc.o: crtc.c cap32.h crtc.h z80.h
	$(CC) $(CFLAGS) $(IPATHS) -c crtc.c

fdc.o: fdc.c cap32.h z80.h
	$(CC) $(CFLAGS) $(IPATHS) -c fdc.c

glfuncs.o: glfuncs.cpp glfuncs.h glfunclist.h
	$(CC) $(CFLAGS) $(IPATHS) -c glfuncs.cpp

psg.o: psg.c cap32.h z80.h
	$(CC) $(CFLAGS) $(IPATHS) -c psg.c

tape.o: tape.c cap32.h tape.h z80.h
	$(CC) $(CFLAGS) $(IPATHS) -c tape.c

video.o: video.cpp video.h
	$(CC) $(CFLAGS) $(IPATHS) -c video.cpp

z80.o: z80.c z80.h cap32.h
	$(CC) $(CFLAGS) $(IPATHS) -c z80.c

clean:
	del .\*.o
	del cap32.exe
