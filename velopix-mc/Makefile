CFLAGS=-g -Wall -O3 -Wno-unused-variable -Wno-unused-result
LIBS=-lm

APPLICATIONS=decode emu rand hists

all: 	$(APPLICATIONS)


%: 	%.c
	gcc  $(CFLAGS) $@.c -o $@ $(LIBS)

hists: hists.cpp 
	g++  $(CFLAGS) $@.cpp -o $@ `root-config --cflags --libs`

emu: emu.cpp 
	g++  $(CFLAGS) $@.cpp -o $@ `root-config --cflags --libs`

clean:
	rm -f *~ $(APPLICATIONS)
