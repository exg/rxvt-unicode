
CFLAGS=-O

first_rule: all

all allbin: ../lib/libW11.dll

alldoc tags:

../lib/libW11.dll: xlib.o ntutil.o event.o
	dllwrap --export-all-symbols -o ../lib/libW11.dll xlib.o ntutil.o event.o -mwindows

xlib.o: xlib.c ntdef.h
	gcc -c xlib.c ${CFLAGS} -I.. -mwin32

ntutil.o: ntutil.c ntdef.h
	gcc -c ntutil.c ${CFLAGS} -I.. -mwin32

event.o: event.c ntdef.h
	gcc -c event.c ${CFLAGS} -I.. -mwin32

debug:
	gcc -c xlib.c -g -I.. -mwin32
	gcc -c event.c -g -I.. -mwin32
	gcc -c ntutil.c -g -I.. -mwin32

clean:
	-rm ../lib/libW11.dll xlib.o ntutil.o event.o
