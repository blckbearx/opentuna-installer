//#define __DEBUG_PRINTF__

/*------------------------------------------------------------*/
#include "main.h"
#include "complete.h"
#include "error.h"
#include "inst.h"
#include "wait.h"
#include "MensajeA.h"
#include "MensajeB.h"
#include "MensajeC.h"
#include "MensajeD.h"
#include "MensajeE.h"
#include "MensajeF.h"
//*/

//----------------------------------------//
extern u8 opentuna_icn[];
extern int size_opentuna_icn;
//----------------------------------------//
extern u8 opentuna_sys[];
extern int size_opentuna_sys;
//----------------------------------------//
extern u8 opl_elf[];
extern int size_opl_elf;
//----------------------------------------//
extern u8 opl_icn[];
extern int size_opl_icn;
//----------------------------------------//
extern u8 opl_sys[];
extern int size_opl_sys;
//----------------------------------------//
extern u8 ule_elf[];
extern int size_ule_elf;
//----------------------------------------//
extern u8 ule_icn[];
extern int size_ule_icn;
//----------------------------------------//
extern u8 ule_sys[];
extern int size_ule_sys;
//----------------------------------------//
extern u8 apps_icn[];
extern int size_apps_icn;
//----------------------------------------//
extern u8 apps_sys[];
extern int size_apps_sys;

static int pad_inited = 0;

//--------------------------------------------------------------
static int file_exists(char *filepath)
{
	int fdn;

	fdn = open(filepath, O_RDONLY);
	if (fdn < 0)
		return 0;

	close(fdn);

	return 1;
}

//--------------------------------------------------------------
static void Reset_IOP(void)
{
	//parrado
	SifInitRpc(0);
	while (!SifIopReset("", 0))
	{
	};
	while (!SifIopSync())
	{
	};
	SifInitRpc(0);
}
//=============================================================
static void display_bmp(u16 W, u16 H, u32 *data)
{
	gs_print_bitmap(
	(gs_get_max_x() - W) / 2,   //x
	(gs_get_max_y() - H) / 2,   //y
	W,                          //w
	H,                          //h
	data                        //array
	);

#ifdef __DEBUG_PRINTF__
	printf("array displayed\n");
#endif
}
//=============================================================

static void InitPS2(void)
{
	Reset_IOP();
	SifInitIopHeap();
	SifLoadFileInit();
	fioInit();

	sbv_patch_disable_prefix_check();
	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	SifLoadModule("rom0:MCMAN", 0, NULL);
	SifLoadModule("rom0:MCSERV", 0, NULL);
	SifLoadModule("rom0:PADMAN", 0, NULL);

	//Faltaba iniciar la MC (alexparrado)
	mcInit(MC_TYPE_MC);

	setupPad();
	waitAnyPadReady();
}

//write &embed_file to path
// returns:
// -1 fail to open | -2 failed to write | 0 succes
static int write_embed(void *embed_file, const int embed_size, char* path, char* filename)
{
	int fd, ret;
	char target[MAX_PATH];

	sprintf(target, "%s/%s", path, filename);
	if ((ret = open(target, O_RDONLY)) < 0)  //if not exist
	{
		if ((fd = open(target, O_CREAT | O_WRONLY | O_TRUNC)) < 0) {
			return -1;  //Failed open
		}
		ret = write(fd, embed_file, embed_size);
		if (ret != embed_size) {
			return -2;  //Failed writing
		}
		close(fd);
	}

#ifdef __DEBUG_PRINTF__
	printf("embed file written: %s\n", target);
#endif

	return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//return 0 = ok, return 1 = error
static int install(void)
{
	int ret, retorno;
	static int mc_Type, mc_Free, mc_Format;

	mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format);
	mcSync(0, NULL, &ret);

	//If there's no MC, we have an error:
	if (ret != -1){return 1;}

	//If it is not a PS2 MC, we have an error:
	//if (mc_Type != 2){return 2;}

	//If there's no free space, we have an error:
	if (mc_Free < 1727){return 3;}

	//If the files exists, we have an error:
	if (file_exists("mc0:/OPENTUNA/icon.icn")) {return 4;}
	if (file_exists("mc0:/OPENTUNA/icon.sys")) {return 4;}
	if (file_exists("mc0:/APPS/icon.sys")) {return 5;}
	if (file_exists("mc0:/APPS/tunacan.icn")) {return 5;}
	if (file_exists("mc0:/BOOT/BOOT.ELF")) {return 5;}
	if (file_exists("mc0:/BOOT/BOOT.icn")) {return 5;}
	if (file_exists("mc0:/BOOT/icon.sys")) {return 5;}
	if (file_exists("mc0:/OPL/OPNPS2LD.ELF")) {return 5;}
	if (file_exists("mc0:/OPL/opl.icn")) {return 5;}
	if (file_exists("mc0:/OPL/icon.sys")) {return 5;}

	ret = mcMkDir(0, 0, "OPENTUNA");
	mcSync(0, NULL, &ret);
	ret = mcMkDir(0, 0, "APPS");
	mcSync(0, NULL, &ret);
	ret = mcMkDir(0, 0, "BOOT");
	mcSync(0, NULL, &ret);
	ret = mcMkDir(0, 0, "OPL");
	mcSync(0, NULL, &ret);

	retorno = write_embed(&opentuna_icn, size_opentuna_icn, "mc0:/OPENTUNA","icon.icn");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&opentuna_sys, size_opentuna_sys, "mc0:/OPENTUNA","icon.sys");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&apps_sys, size_apps_sys, "mc0:/APPS","icon.sys");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&apps_icn, size_apps_icn, "mc0:/APPS","tunacan.icn");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&ule_elf, size_ule_elf, "mc0:/BOOT","BOOT.ELF");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&ule_icn, size_ule_icn, "mc0:/BOOT","BOOT.icn");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&ule_sys, size_ule_sys, "mc0:/BOOT","icon.sys");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&opl_elf, size_opl_elf, "mc0:/OPL","OPNPS2LD.ELF");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&opl_icn, size_opl_icn, "mc0:/OPL","opl.icn");
	if (retorno < 0) {return 6;}
	retorno = write_embed(&opl_sys, size_opl_sys, "mc0:/OPL","icon.sys");
	if (retorno < 0) {return 6;}

#ifdef __DEBUG_PRINTF__
	printf("installation finished\n");
#endif

	static sceMcTblGetDir mcDirAAA[64] __attribute__((aligned(64)));
	static sceMcStDateTime maximahora; //Maxium Timestamp, for the ones who does not speak Spanish

	maximahora.Resv2 = 0;
	maximahora.Sec = 59;
	maximahora.Min = 59;
	maximahora.Hour = 23;
	maximahora.Day = 31;
	maximahora.Month = 12;
	maximahora.Year = 2099;
	mcDirAAA->_Modify = maximahora;
	mcDirAAA->_Create = maximahora;
	mcSetFileInfo(0, 0, "OPENTUNA", mcDirAAA, 0x02);
	mcSync(0, NULL, &ret);

#ifdef __DEBUG_PRINTF__
	printf("timestamp changed\n");
#endif

	return 0;
}
//--------------------------------------------------------------

static void CleanUp(void) //trimmed from FMCB
{
	if (pad_inited) {
		padPortClose(0,0);
		padPortClose(1,0);
		padEnd(); 
	}

	Reset_IOP();

	// Reloads common modules
	SifLoadFileInit();
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();

	SifLoadModule("rom0:SIO2MAN", 0, 0);
	SifLoadModule("rom0:CDVDFSV", 0, 0);
	SifLoadModule("rom0:CDVDMAN", 0, 0);
	SifLoadModule("rom0:MCMAN", 0, 0);
	SifLoadModule("rom0:MCSERV", 0, 0);
	SifLoadModule("rom0:PADMAN", 0, 0);

	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();

	FlushCache(0);
	FlushCache(2);

	// clear the screen
	gs_set_fill_color(0, 0, 0);
	gs_fill_rect(0, 0, gs_get_max_x(), gs_get_max_y());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//close program and go to browser
static void PS2_browser(void)
{
	CleanUp();
	LoadExecPS2("rom0:OSDSYS", 0, NULL);
}

int main (int argc, char *argv[])
{
	int fdn;
	char romver[16];
	VMode = NTSC;

	// Loads Needed modules
	InitPS2();

	gs_reset(); // Reset GS
	if((fdn = open("rom0:ROMVER", O_RDONLY)) > 0) // Reading ROMVER
	{
		read(fdn, romver, sizeof romver);
		close(fdn);

		if (romver[4] == 'E')
			VMode = PAL;
	}

	if (VMode == PAL)
		gs_init(PAL_640_512_32);
	else
		gs_init(NTSC_640_448_32);

	display_bmp(640, 448, inst);

	waitAnyPadReady();
	pad_inited = 1;

	int iz = 1;
	int menuactual = 101;//101: Initial Menu, 102: Installing (not needed), 103: Error, 104: Done

	display_bmp(640, 448, inst);//Again, just in case of an old japanese console
	while (1) {
		readPad();

		if (((new_pad & PAD_CROSS) && (menuactual == 101)) || ((new_pad & PAD_CIRCLE) && (menuactual == 101))) {
			menuactual = 102;
			display_bmp(640, 448, wait);
#ifdef __DEBUG_PRINTF__
			printf("begin install\n");
#endif
			iz = install();
			if(iz == 0){
				menuactual = 104;
				display_bmp(640, 448, complete);
			}
			else {
				menuactual = 103;
				display_bmp(640, 448, error);
				if(iz == 1){gs_print_bitmap(192, 343, 256, 40, MensajeA);}
				else if(iz == 2){gs_print_bitmap(192, 343, 256, 40, MensajeB);}
				else if(iz == 3){gs_print_bitmap(192, 343, 256, 40, MensajeC);}
				else if(iz == 4){gs_print_bitmap(192, 343, 256, 40, MensajeD);}
				else if(iz == 5){gs_print_bitmap(192, 343, 256, 40, MensajeE);}
				else if(iz == 6){gs_print_bitmap(192, 343, 256, 40, MensajeF);}
			}
		}

		else if ((new_pad & PAD_START) && (menuactual == 103)) {PS2_browser();}
		else if ((new_pad & PAD_START) && (menuactual == 104)) {PS2_browser();}
	}

	return 0;
}
