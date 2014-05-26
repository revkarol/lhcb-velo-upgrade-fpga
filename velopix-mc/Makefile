CFLAGS=-g -Wall -O3 -Wno-unused-variable

APPLICATIONS=decode emu rand hists

all: 	$(APPLICATIONS)


%: 	%.c
	gcc  $(CFLAGS) $@.c -o $@

hists: hists.cpp 
	g++  $(CFLAGS) $@.cpp -o $@ `root-config --cflags --libs`


clean:
	rm -f *~ $(APPLICATIONS)
