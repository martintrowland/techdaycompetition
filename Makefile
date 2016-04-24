all: galaxy robot

src = $(wildcard *.c)
obj = $(src:.c=.o)
log = $(wildcard *.log)
exe = $(wildcard *.exe)

IFLAGS =-I/usr/X11R6/include -I/usr/X11R6/include/X11
LFLAGS = -L/usr/X11R6/lib -L/usr/X11R6/lib/X11 -lX11 -lm

galaxy: galaxy.o viewer.o
	$(CC) -g -o $@ $^ $(IFLAGS) $(LFLAGS)

robot: robot.o
	$(CC) -g -o $@ $^ $(IFLAGS) $(LFLAGS)

run: galaxy robot
	./galaxy 10 10 5 0 ./robot Nikhil ./robot Daniel ./robot Mark

.PHONY: clean
clean:
	rm -f $(obj) $(log) $(exe)
