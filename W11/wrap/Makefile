
CFLAGS=-O

first_rule: all

all allbin: ../lib/libX11.a

alldoc tags:

../lib/libX11.a: wrap.o rxvt_res.o
	ar r ../lib/libX11.a wrap.o rxvt_res.o

wrap.o: wrap.c xwrappers.gen
	gcc ${CFLAGS} -c wrap.c -I.. -mwin32

rxvt_res.o: rxvt.rc
	windres -i rxvt.rc -o rxvt_res.o

xwrappers.gen: ../X11/Xlib.h
	perl mkproxy.pl ../X11/Xlib.h > xwrappers.gen

clean:
	-rm wrap.o rxvt_res.o ../lib/libX11.a 
