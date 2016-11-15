# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g 

# the build target executable:

HEADERS = vpx.h 

default: vpserver vpclient

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS)

vpserver : $(OBJECTS)
	gcc $@.o -o $@

vpclient: $(OBJECTS)
	gcc $@.o -o $@
clean:
	-rm -f *.o
	-rm -f vpserver vpclient


