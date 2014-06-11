######################################################################## 
# $Id:: makefile 1031 2008-08-06 22:28:49Z wellsk                      $
# 
# Project: Bootable/FLASHable example makefile
# 
# Description: 
#  Makefile
# 
######################################################################## 
# Software that is described herein is for illustrative purposes only  
# which provides customers with programming information regarding the  
# products. This software is supplied "AS IS" without any warranties.  
# NXP Semiconductors assumes no responsibility or liability for the 
# use of the software, conveys no license or title under any patent, 
# copyright, or mask work right to the product. NXP Semiconductors 
# reserves the right to make changes in the software without 
# notification. NXP Semiconductors also make no representation or 
# warranty that such application will be suitable for the specified 
# use without further testing or modification. 
########################################################################

EXECNAME    =kickstart

########################################################################
#
# Pick up the default build rules 
#
########################################################################

include $(NXPMCU_SOFTWARE)/makerule/$(CSP)/make.$(CSP).$(TOOL)

########################################################################
#
# Arm compiler/linker specific stuff
#
# ARM examples enter via __main and are linked at address 0x0000
#
########################################################################

ifeq ($(TOOL), ads)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-g
AFLAGS      +=-g
LDSCRIPT    =linker/ldscript_rom_$(TOOL).ld
endif
ifeq ($(TOOL), rvw)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-g
AFLAGS      +=-g
LDSCRIPT    =linker/ldscript_rom_$(TOOL).ld
endif
ifeq ($(TOOL), keil)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-g
AFLAGS      +=-g
LDSCRIPT    =linker/ldscript_rom_rvw.ld
endif

########################################################################
#
# Gcc compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), gnu)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-gdwarf-2
AFLAGS      +=
LDSCRIPT    =linker/ldscript_rom_$(TOOL).ld
endif

########################################################################
#
# Pick up the assembler and C source files in the directory  
#
########################################################################
include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.ftypes

########################################################################
#
# Rules to build the executable 
#
########################################################################

default: release

release:	
	@$(MAKE) TOOL=$(TOOL) debug
	@$(MAKE) TOOL=$(TOOL) bin
	@$(MAKE) TOOL=$(TOOL) hex

debug: $(OBJS) lpc_libs
	$(LD) $(OBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) $(MAPFILE)$(MEXT) \
	$(LDESC) $(LDSCRIPT) $(ENTRY) $(C_ENTRY) -o $(EXECNAME)$(EXT)

bin: 
	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EXECNAME).bin

hex:
ifeq ($(TOOL), ads)
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME).$(REC)
endif

ifeq ($(TOOL), rvw)
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME).$(REC)
endif

ifeq ($(TOOL), ghs)
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME).$(REC)
endif

ifeq ($(TOOL), gnu)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME).$(REC)
endif

ifeq ($(TOOL), iar)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME).$(REC)
endif

########################################################################
#
# Pick up the compiler and assembler rules
#
########################################################################

include $(NXPMCU_SOFTWARE)/makerule/common/make.rules.build

.PHONY: default release debug bin

