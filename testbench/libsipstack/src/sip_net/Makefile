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

CC_PWD     =$(shell pwd)
LD_TARGET  =$(CC_BUILTIN)
LD_FLAGS  +=
CC_INCLUDE+= $(wildcard $(CC_PWD)include/*.h)

#file need to be CC
INC_FILES = $(CC_INCLUDE) $(wildcard *.h) 
ASM_FILES = $(wildcard *.S) 
SRC_FILES = $(wildcard *.c)
CPP_FILES = $(wildcard *.cc)
SRC_DIRS  = linux

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
