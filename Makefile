EE_BIN = Installer.elf
EE_BIN_PACKED = OpenTuna_Installer.elf
EE_BIN_STRIPPED = stripped.elf
EE_OBJS = main.o gs.o pad.o gs_asm.o ps2_asm.o dma_asm.o
EE_OBJS += opentuna_icn.o opentuna_sys.o opl_elf.o opl_icn.o opl_sys.o ule_elf.o ule_icn.o ule_sys.o apps_icn.o apps_sys.o
EE_SRC = opentuna_icn.s opentuna_sys.s opl_elf.s opl_icn.s opl_sys.s ule_elf.s ule_icn.s ule_sys.s apps_icn.s apps_sys.s
EE_LIBS = -ldebug -lcdvd -lpatches -lpad -lmc

all:
	$(MAKE) $(EE_BIN_PACKED)

opentuna_icn.s:
	bin2s INSTALL/OPENTUNA/icon.icn opentuna_icn.s opentuna_icn

opentuna_sys.s:
	bin2s INSTALL/OPENTUNA/icon.sys opentuna_sys.s opentuna_sys

opl_elf.s:
	bin2s INSTALL/OPL/OPNPS2LD.ELF opl_elf.s opl_elf

opl_icn.s:
	bin2s INSTALL/OPL/opl.icn opl_icn.s opl_icn

opl_sys.s:
	bin2s INSTALL/OPL/icon.sys opl_sys.s opl_sys

ule_elf.s:
	bin2s INSTALL/BOOT/BOOT.ELF ule_elf.s ule_elf

ule_icn.s:
	bin2s INSTALL/BOOT/BOOT.icn ule_icn.s ule_icn

ule_sys.s:
	bin2s INSTALL/BOOT/icon.sys ule_sys.s ule_sys

apps_icn.s:
	bin2s INSTALL/APPS/tunacan.icn apps_icn.s apps_icn

apps_sys.s:
	bin2s INSTALL/APPS/icon.sys apps_sys.s apps_sys
	
clean:
	rm -fr *.o $(EE_BIN_PACKED) $(EE_BIN_STRIPPED) $(EE_BIN) opentuna_icn.* opentuna_sys.* opl_elf.* opl_icn.* opl_sys.* ule_elf.* ule_icn.* ule_sys.* apps_icn.* apps_sys.*

$(EE_BIN_STRIPPED): $(EE_BIN)
	$(EE_STRIP) -o $@ $<

$(EE_BIN_PACKED): $(EE_BIN_STRIPPED)
	ps2-packer -v $< $@

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
