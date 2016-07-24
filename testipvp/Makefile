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

##############################################################################
##############################################################################

##add all libs
TESTCASE_LIBS += $(CC_OUTPUT)/$(CC_CROSS)libtestbench.o

##############################################################################
##############################################################################
CC_HOST  = $(CC_CROSS)g++
LD_HOST  = $(CC_CROSS)ld
AR_HOST  = $(CC_CROSS)ar
ST_HOST  = $(CC_CROSS)strip

CC_FLAGS+= $(TESTCASE_INCL)
CC_FLAGS+= -I$(CC_PWD)
CC_FLAGS+= -I$(CC_PWD)/testbench/include/
CC_FLAGS+= -I$(CC_PWD)/testipvp/ipvp/include/
CC_FLAGS+= -I$(CC_PWD)/testipvp/msgkit/include/

LD_FLAGS+= -Wl,-rpath,:./:/phone/lib/:/boot/lib/
LD_FLAGS+= -L$(CC_PWD)/out/
LD_FLAGS+= -lpthread -lresolv
LD_FLAGS+= $(LD_LIBS)
LD_TARGET=
LD_DEV   =
LD_VER   =
LD_EXT   =

LD_NAME  = runipvp
LD_VER   = 

LD_TARGET  =$(LD_NAME)

# symbol export
export CC_HOST
export LD_HOST
export AR_HOST
export ST_HOST
export LD_FLAGS
export CC_FLAGS

#file need to be CC
INC_FILES = $(wildcard *.h)
SRC_FILES = $(wildcard *.cpp)
SRC_DIRS  = 

C_FILES		=$(SRC_FILES)
O_FILES		=$(C_FILES:.cpp=.o)

DIRS_CLEAN =$(SRC_DIRS:=_clean)
DIRS_BUILD =$(SRC_DIRS:=_all)
DIRS_OBJECT=$(SRC_DIRS:=/$(CC_BUILTIN))

###########################################################################
# for target
###########################################################################
all :  $(LD_TARGET)
	@mv $(LD_TARGET) $(CC_OUTPUT)/
	@echo "BUILD   done"
	
clean: $(DIRS_CLEAN)
	@-rm -rf  *.o
	@-rm -rf  $(O_FILES)
	@-rm -rf  $(LD_TARGET)
	@-rm -rf  ../$(LD_TARGET)
	@echo "CLEAN   done"

$(DIRS_BUILD) :
	@echo "BUILD   $(@:_all=) ..."
	@$(MAKE) -C $(@:_all=)   -f $(CC_MKFILE) all

$(DIRS_CLEAN) :
	@echo "CLEAN   $(@:_clean=) ..."
	@$(MAKE) -C $(@:_clean=) clean --no-print-directory

#
# objs
#	
%.o: %.cpp $(INC_FILES)
	@echo "CC      $(@:.o=.cpp)"
	$(CC_ECHO) $(CC_HOST) -c $(@:.o=.cpp) $(CC_FLAGS) -I$(dir $(@))  -o $@
	
#
# target
#	
$(LD_TARGET): $(O_FILES) $(TO_FILES) $(DIRS_BUILD)
	@echo "LD      $(LD_TARGET)"
	$(CC_ECHO)$(CC_HOST) -o $(@) $(O_FILES) $(TO_FILES) $(DIRS_OBJECT) $(TESTCASE_LIBS) $(LD_FLAGS) 
	

.PHONY :clean

