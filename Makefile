CC    = gcc
C++   = c++
CPP   = gcc
AR   =  ar -rs 
#DEBUG="OFF"


#Options passed to the C-Compiler
IOPT = 
LOPT = -lz -lpthread

ifdef DEBUG
COPT = -c -g3  $(IOPT)
LIBS = -lstdc++ 
else
COPT = -c -O3  -fexpensive-optimizations -funroll-loops -fomit-frame-pointer  $(IOPT)
LIBS = -lstdc++ 
endif


CSRC= cell.cpp dna.cpp measurement.cpp nacentdnasimul.cpp threadcontrol.cpp \
 threadvariablesclient.cpp replicator.cpp thread.cpp threadvariables.cpp \
 dataBasic.cc dataIF.cpp dataIO.cc uiParms.cpp gzstream.cc tabDelimited.cc

repliconobj= cell.o dna.o measurement.o nacentdnasimul.o threadcontrol.o \
 threadvariablesclient.o replicator.o thread.o threadvariables.o \
 dataBasic.o dataIF.o dataIO.o uiParms.o gzstream.o tabDelimited.o



exec= replicon

default: all

all:	$(exec)

replicon:	$(repliconobj)
	$(C++) $(LOPT) -o replicon $(repliconobj) $(LIBS)   

######################################################
# Some general rules                                 #
######################################################
.c.o:	
	$(CC)  $(COPT) $<

.cpp.o:
	$(C++)  $(COPT) $<

.cc.o:
	$(C++)  $(COPT) $<

dep:	$(CSRC) 
	$(CPP) -M $(IOPT)  $(CSRC) -Wno-deprecated > .depend

clean:
	rm -f *.o  core *~ ./#*


distclean:	clean
	rm -f ${exec}


include .depend	
















