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
#for gcc
CC_PLATFORM = \
              mipsel-linux-uclibc- \
              arm_v5t_le- \
              arm-linux-gnueabi- \
              arm-none-linux-gnueabi- \
              armv6jel- \
              "" 

#for pwd
CC_PWD	    =$(shell pwd)

#for output
CC_OUTPUT   =$(CC_PWD)/out

#for cc flags
CC_FLAGS    += $(CC_DEBUG)
#for export headers
CC_FLAGS    += -I./ -I$(CC_PWD)

#for CC
CC_FLAGS    += -Wall -o2 #-pg
#for ld flags
LD_FLAGS	  +=

#for include header file
CC_INCLUDE  += 

##################################################################
# platform setting
##################################################################
#for math lib
CC_FLAGS  += -DHAVE_LIBRESOLV
CC_FLAGS  += -DHAVE_INTTYPES_H

#for cross
ifeq ($(CROSS),)
ALL	=	test_lib
ALSA= YES
else
ALL	=	libs
endif

##################################################################
# debug setting
##################################################################
ifeq ($(CROSS),)
all_action=test_lib
else
all_action=libs
endif
# for CC echo
ifeq ($(ECHO),YES)
CC_ECHO    =
else
CC_ECHO    =@
endif

#for debug
CC_DEBUG   =
ifeq ($(DEBUG),YES)
	CC_FLAGS+=-D_DEBUG
	CC_FLAGS+=-DDSP_DEBUG
endif

# for CC ALSA
ifeq ($(ALSA),YES)
LD_LIBS += -lasound 
CC_FLAGS+= -DHAVE_ALSA
CC_ALSA  = YES
endif

# for CC pcap
ifeq ($(PCAP),YES)
CC_FLAGS += -DHAVE_PCAP
CC_PCAP   = YES
endif

#for builtin
CC_BUILTIN = built-in.o

# Do not print "Entering directory ..."
MAKEFLAGS += --no-print-directory

###########################################################################
# for compile compatible
###########################################################################
#
# compatible for newer than version on 2.6.30
#

###########################################################################
# for export
###########################################################################
export CC_CROSS
export CC_PWD
export CC_BUILTIN
export CC_OUTPUT
export CC_FLAGS
export LD_FLAGS
export LD_LIBS
export CC_ECHO
export CC_INCLUDE

export CC_ALSA
export CC_PCAP

# all common to build
CC_ALL_SUBDIR    =testbench
CC_ALL_BUILD     =$(CC_ALL_SUBDIR:=_all)
CC_ALL_CLEAN     =$(CC_ALL_SUBDIR:=_clean)
###########################################################################
#
###########################################################################

all: show $(ALL)
	@echo "BUILD   done"

libs: check_libs $(CC_ALL_BUILD)
	@echo "BUILD   libs done"

###########################################################################
#
###########################################################################

test:show check_libs test_lib test_main
	@echo "Build Done!"
	
ipvp:show check_libs test_lib test_ipvp
	@echo "Build Done!"
	
test_lib:
	@$(MAKE) -C testbench -f Makefile CC_CROSS=$(CROSS) --no-print-directory

test_main:test_lib
	@$(MAKE) -C testmain  -f Makefile CC_CROSS=$(CROSS) --no-print-directory

test_ipvp:test_lib
	@$(MAKE) -C testipvp  -f Makefile CC_CROSS=$(CROSS) --no-print-directory

###########################################################################
#
###########################################################################

clean: clean_test clean_main
	@echo "Clean Done!"

clean_test:
	@echo "CLEAN   testbench"
	@$(MAKE) -C testbench -f Makefile  clean --no-print-directory
	
clean_main:
	@echo "CLEAN   testmain"
	@$(MAKE) -C testmain  -f Makefile  clean --no-print-directory

clean_ipvp:
	@echo "CLEAN   testipvp"
	@$(MAKE) -C testipvp  -f Makefile  clean --no-print-directory

###########################################################################
#
###########################################################################	

show:
	@echo "CROSS =$(CC_CROSS)"
	@echo "PWD   =$(CC_PWD)"
	
install:
	@echo "install to no where..."

check_libs:
	@echo "check done..."


###########################################################################
#
###########################################################################			
$(CC_ALL_BUILD) :
	@echo "BUILD   $(@:_all=)"
	@for platform in $(CC_PLATFORM); \
	do \
	 echo "";\
	 echo "make by $$platform gcc  for linux";\
	 $(MAKE) -C $(@:_all=) -f Makefile CC_CROSS=$$platform --no-print-directory; \
	done 

$(CC_ALL_CLEAN) :
	@echo "CLEAN   $(@:_clean=)"	
	@(cd $(@:_clean=) ;$(MAKE) clean)

###########################################################################
#
###########################################################################	
help:
	@echo "=========================================="
	@echo "help for building"
	@echo "   "
	@echo "build test:"
	@echo ""
	@echo "	make test CROSS=??"
	@echo "	make ipvp CROSS=??"
	@echo ""
	@echo "for link options"
	@echo "	make ?? ALSA=YES PCAP=YES"
	@echo ""	
	@echo "for compile debug"
	@echo "	make ?? ECHO=YES"
	@echo ""
	@echo "support platform:"
	@	@for platform in $(CC_PLATFORM); \
	do echo "   $$platform";done 
	@echo ""		
	@echo "=========================================="	

