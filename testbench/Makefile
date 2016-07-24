#/********************************************************************
#*
#*    DESCRIPTION:
#*
#*    AUTHOR:
#*
#*    HISTORY:
#*
#*    DATE:2011-03-26
#*
#*******************************************************************/
CC_HOST  = $(CC_CROSS)gcc
LD_HOST  = $(CC_CROSS)ld
AR_HOST  = $(CC_CROSS)ar
ST_HOST  = $(CC_CROSS)strip
SO_HOST  = $(CC_CROSS)gcc -fPIC -shared

#for LD
LD_TARGET  = $(CC_OUTPUT)/$(CC_CROSS)libtestbench.o
LD_FLAGS  += 

#for pwd
CC_PWD     =$(shell pwd)

#for CC
CC_FLAGS  += -I$(CC_PWD)/
CC_FLAGS  += -I$(CC_PWD)/include/
CC_FLAGS  += -I$(CC_PWD)/source/
CC_FLAGS  += -I$(CC_PWD)/source/platform/
CC_FLAGS  += -I$(CC_PWD)/libsipstack/include
CC_FLAGS  += -I$(CC_PWD)/libpcap/include
CC_FLAGS  += -I$(CC_PWD)/libpcap/include/pcap
CC_INCLUDE+= $(wildcard $(CC_PWD)/*.h)
CC_INCLUDE+= $(wildcard $(CC_PWD)/host/*.h)

CC_BUILTIN= build-in.o
CC_MKFILE = Makefile


###########################################################################
# for export
###########################################################################
export CC_CROSS
export CC_PWD
export CC_BUILTIN
export CC_OUTPUT
export CC_FLAGS
export LD_FLAGS
export CC_ECHO
export CC_ROOT
export CC_MKFILE
export CC_INCLUDE

export CC_HOST
export LD_HOST
export AR_HOST
export ST_HOST
export SO_HOST

###########################################################################
# for source
###########################################################################
#file need to be CC
INS_FILE  = $(CC_INCLUDE) $(wildcard *.h)
ASM_FILES = $(wildcard *.S) 
SRC_FILES = $(wildcard *.c)
CPP_FILES = $(wildcard *.cc)
SRC_DIRS  = source libpcap libsipstack

A_FILES	  =$(ASM_FILES)
C_FILES	  =$(SRC_FILES)
P_FILES	  =$(CPP_FILES)
AO_FILES  =$(A_FILES:.S=.o)
CO_FILES  =$(C_FILES:.c=.o)
PO_FILES  =$(P_FILES:.cc=.o)

DIRS_CLEAN =$(SRC_DIRS:=_clean)
DIRS_BUILD =$(SRC_DIRS:=_all)
DIRS_OBJECT=$(SRC_DIRS:=/$(CC_BUILTIN))

all : $(LD_TARGET)

clean: $(DIRS_CLEAN) 
	@-rm -rf  *.o
	@-rm -rf  $(LD_TARGET)
	
$(DIRS_BUILD) :
	@echo "BUILD   $(@:_all=) ..."
	@$(MAKE) -C $(@:_all=)   -f $(CC_MKFILE) all

$(DIRS_CLEAN) :
	@echo "CLEAN   $(@:_clean=) ..."
	@$(MAKE) -C $(@:_clean=) -f $(CC_MKFILE) clean

#
# objs
#	
$(PO_FILES) : %.o : %.cc  $(INC_FILES)
	@echo "CC      $(@:.o=.cc)"
	$(CC_ECHO)$(CC_HOST) -c $(@:.o=.cc) $(CC_FLAGS)  -o $@

$(CO_FILES) : %.o : %.c  $(INC_FILES)
	@echo "CC      $(@:.o=.c)"
	$(CC_ECHO)$(CC_HOST) -c $(@:.o=.c) $(CC_FLAGS)  -o $@

$(AO_FILES) : %.o : %.S  $(INC_FILES)
	@echo "AC      $(@:.o=.S)"
	$(CC_ECHO)$(CC_HOST) -c $(@:.o=.S) $(CC_FLAGS) -D__ASSEMBLY__ -o $@

#
# target
#	
$(LD_TARGET) : $(DIRS_BUILD) $(PO_FILES) $(CO_FILES) $(AO_FILES)
	@echo "LD      $(LD_TARGET)"
	$(CC_ECHO)$(LD_HOST) $(LD_FLAGS) -r $(DIRS_OBJECT) $(PO_FILES) $(CO_FILES) $(AO_FILES) -o $(LD_TARGET)
	
.PHONY :clean	
