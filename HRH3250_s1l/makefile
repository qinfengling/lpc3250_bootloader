######################################################################## 
# $Id:: makefile 875 2008-07-08 17:27:04Z wellsk                       $
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

EXECNAME    =stage1

########################################################################
#
# Pick up the default build rules 
#
########################################################################
include $(NXPMCU_SOFTWARE)/makerule/$(CSP)/make.$(CSP).$(TOOL)
VPATH = ../../;$(NXPMCU_SOFTWARE)/ip/s1l/source;

########################################################################
#
# Arm compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), rvw)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-g -I../../ -I$(NXPMCU_SOFTWARE)/ip/s1l/include
AFLAGS      +=-g -I../../
LDSCRIPT    =../linker/ldscript_rom_$(TOOL).ld
endif
ifeq ($(TOOL), keil)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-g -I../../ -I$(NXPMCU_SOFTWARE)/ip/s1l/include
AFLAGS      +=-g -I../../
LDSCRIPT    =../linker/ldscript_rom_$(TOOL).ld
endif

########################################################################
#
# Pick up the assembler and C source files in the directory  
#
########################################################################
include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.ftypes

########################################################################
#
# Pick up common startup files 
#
########################################################################
OBJS += phy3250_startup.o phy3250_startup_entry.o

########################################################################
#
# Pick up S1L core files 
#
########################################################################
OBJS += s1l_line_input.o s1l_memtests.o s1l_movdat.o s1l_except.o
OBJS += s1l_cmds.o s1l_cmds_core.o s1l_sys.o s1l_bootmgr.o s1l_fat.o
OBJS += s1l_cmds_image.o s1l_cmds_flash.o s1l_image_mngt.o

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
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME).lolo$(REC)
endif

ifeq ($(TOOL), rvw)
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME).lolo$(REC)
endif

ifeq ($(TOOL), ghs)
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME).lolo$(REC)
endif

ifeq ($(TOOL), gnu)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME).lolo$(REC)
endif

ifeq ($(TOOL), iar)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME).lolo$(REC)
endif

########################################################################
#
# Pick up the compiler and assembler rules
#
########################################################################

include $(NXPMCU_SOFTWARE)/makerule/common/make.rules.build

.PHONY: default release test debug debugtest bin

