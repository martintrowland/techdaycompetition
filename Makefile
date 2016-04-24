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
	./galaxy -x 10 -y 5 -neutrals 5 -bots ./robot Nikhil ./robot Daniel ./robot Mark

.PHONY: clean
clean:
	rm -f $(obj) $(log) $(exe)
