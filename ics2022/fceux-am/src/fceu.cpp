/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "ppu.h"
#include "sound.h"
#include "file.h"
#include "utils/memory.h"

#include "cart.h"
#include "ines.h"
#include "palette.h"
#include "state.h"
#include "video.h"
#include "input.h"
#include "file.h"
#include "ines.h"

extern void RefreshThrottleFPS();

#include "drivers/sdl/sdl.h"

using namespace std;

//-----------
// overclocking-related
// overclock the console by adding dummy scanlines to PPU loop or to vblank
// disables DMC DMA, WaveHi filling and image rendering for these dummies
// doesn't work with new PPU
bool overclock_enabled = 0;
bool overclocking = 0;
bool skip_7bit_overclocking = 1; // 7-bit samples have priority over overclocking
int normalscanlines;
int totalscanlines;
int postrenderscanlines = 0;
int vblankscanlines = 0;
//------------

// If this is true, frame advance will skip over lag frame (i.e. it will emulate 2 frames instead of 1)
#define frameAdvanceLagSkip false

FCEUGI::FCEUGI() {}

FCEUGI::~FCEUGI() {}

void FCEU_TogglePPU(void)
{
	newppu ^= 1;
	if (newppu)
	{
		FCEU_DispMessage("New PPU loaded");
		FCEUI_printf("New PPU loaded");
		overclock_enabled = 0;
	}
	else
	{
		FCEU_DispMessage("Old PPU loaded");
		FCEUI_printf("Old PPU loaded");
	}
	normalscanlines = (dendy ? 290 : 240) + newppu; // use flag as number!
}

static void FCEU_CloseGame(void)
{
	if (GameInfo)
	{
		if (GameInfo->name)
		{
			free(GameInfo->name);
			GameInfo->name = NULL;
		}

		if (GameInfo->type != GIT_NSF)
		{
			// FCEU_FlushGameCheats(0, 0);
		}

		GameInterface(GI_CLOSE);

		// FCEUI_StopMovie();

		// ResetExState(0, 0);

		// clear screen when game is closed

		// for (int i = 0; i < 256; i++)
		// {
		// 	for (int j = 0; j < 256; j++)
		// 		printf("%d ", XBuf[i * 256 + j]);
		// 	printf("\n");
		// }
		extern uint8 *XBuf;
		if (XBuf)
			memset(XBuf, 0, 256 * 256);

		FCEU_CloseGenie();

		free(GameInfo);
		GameInfo = NULL;
	}
}

uint64 timestampbase;

FCEUGI *GameInfo = NULL;

void (*GameInterface)(GI h);
void (*GameStateRestore)(int version);

readfunc ARead[0x10000];
writefunc BWrite[0x10000];
static readfunc *AReadG;
static writefunc *BWriteG;
static int RWWrap = 0;

// mbg merge 7/18/06 docs
// bit0 indicates whether emulation is paused
// bit1 indicates whether emulation is in frame step mode
int EmulationPaused = 0;
bool frameAdvanceRequested = false;
int frameAdvance_Delay_count = 0;
int frameAdvance_Delay = FRAMEADVANCE_DELAY_DEFAULT;

int AutosaveQty = 4;		 // Number of Autosaves to store
int AutosaveFrequency = 256; // Number of frames between autosaves

// Flag that indicates whether the Auto-save option is enabled or not
int EnableAutosave = 0;

static DECLFW(BNull)
{
}

static DECLFR(ANull)
{
	return (X.DB);
}

int AllocGenieRW(void)
{
	if (!(AReadG = (readfunc *)FCEU_malloc(0x8000 * sizeof(readfunc))))
		return 0;
	if (!(BWriteG = (writefunc *)FCEU_malloc(0x8000 * sizeof(writefunc))))
		return 0;
	RWWrap = 1;
	return 1;
}

void FlushGenieRW(void)
{
	int32 x;

	if (RWWrap)
	{
		for (x = 0; x < 0x8000; x++)
		{
			ARead[x + 0x8000] = AReadG[x];
			BWrite[x + 0x8000] = BWriteG[x];
		}
		free(AReadG);
		free(BWriteG);
		AReadG = NULL;
		BWriteG = NULL;
		RWWrap = 0;
	}
}

readfunc GetReadHandler(int32 a)
{
	if (a >= 0x8000 && RWWrap)
		return AReadG[a - 0x8000];
	else
		return ARead[a];
}

void SetReadHandler(int32 start, int32 end, readfunc func)
{
	int32 x;

	if (!func)
		func = ANull;

	if (RWWrap)
		for (x = end; x >= start; x--)
		{
			if (x >= 0x8000)
				AReadG[x - 0x8000] = func;
			else
				ARead[x] = func;
		}
	else
		for (x = end; x >= start; x--)
			ARead[x] = func;
}

writefunc GetWriteHandler(int32 a)
{
	if (RWWrap && a >= 0x8000)
		return BWriteG[a - 0x8000];
	else
		return BWrite[a];
}

void SetWriteHandler(int32 start, int32 end, writefunc func)
{
	int32 x;

	if (!func)
		func = BNull;

	if (RWWrap)
		for (x = end; x >= start; x--)
		{
			if (x >= 0x8000)
				BWriteG[x - 0x8000] = func;
			else
				BWrite[x] = func;
		}
	else
		for (x = end; x >= start; x--)
			BWrite[x] = func;
}

uint8 RAM[0x800];

//------

uint8 PAL = 0;

static DECLFW(BRAML)
{
	RAM[A] = V;
}

static DECLFW(BRAMH)
{
	RAM[A & 0x7FF] = V;
}

static DECLFR(ARAML)
{
	return RAM[A];
}

static DECLFR(ARAMH)
{
	return RAM[A & 0x7FF];
}

void ResetGameLoaded(void)
{
	if (GameInfo)
		FCEU_CloseGame();
	EmulationPaused = 0; // mbg 5/8/08 - loading games while paused was bad news. maybe this fixes it
	GameStateRestore = 0;
	PPU_hook = NULL;
	GameHBIRQHook = NULL;
	FFCEUX_PPURead = NULL;
	FFCEUX_PPUWrite = NULL;
	if (GameExpSound.Kill)
		GameExpSound.Kill();
	memset(&GameExpSound, 0, sizeof(GameExpSound));
	MapIRQHook = NULL;
	MMC5Hack = 0;
	PEC586Hack = 0;
	QTAIHack = 0;
	PAL &= 1;
	default_palette_selection = 0;
}

int iNESLoad(const char *name, FCEUFILE *fp, int OverwriteVidMode);

// name should be UTF-8, hopefully, or else there may be trouble
FCEUGI *FCEUI_LoadGameVirtual(const char *name, int OverwriteVidMode, bool silent)
{
	//----------
	// attempt to open the files
	FCEUFILE *fp;
	char fullname[2048]; // this name contains both archive name and ROM file name
	int lastpal = PAL;
	int lastdendy = dendy;

	const char *romextensions[] = {"nes", "fds", 0};

	// indicator for if the operaton was canceled by user
	// currently there's only one situation:
	// the user clicked cancel form the open from archive dialog
	int userCancel = 0;
	fp = FCEU_fopen(name, 0, "rb", 0, -1, romextensions, &userCancel);
	// printf("===%p===\n", fp);
	if (!fp)
	{
		// Although !fp, if the operation was canceled from archive select dialog box, don't show the error message;
		if (!silent && !userCancel)
			FCEU_PrintError("Error opening \"%s\"!", name);

		return 0;
	}
	else
		strcpy(fullname, name);

	// reset loaded game BEFORE it's loading.
	ResetGameLoaded();
	// file opened ok. start loading.
	FCEU_printf("Loading ...\n\n");

	FCEU_CloseGame();
	GameInfo = (FCEUGI *)malloc(sizeof(FCEUGI));
	memset(GameInfo, 0, sizeof(FCEUGI));

	GameInfo->archiveCount = fp->archiveCount;

	GameInfo->soundchan = 0;
	GameInfo->soundrate = 0;
	GameInfo->name = 0;
	GameInfo->type = GIT_CART;
	GameInfo->vidsys = GIV_USER;
	GameInfo->input[0] = GameInfo->input[1] = SI_UNSET;
	GameInfo->inputfc = SIFC_UNSET;
	GameInfo->cspecial = SIS_NONE;

	// try to load each different format
	bool FCEUXLoad(const char *name, FCEUFILE *fp);

	if (iNESLoad(fullname, fp, OverwriteVidMode))
	{
		if (OverwriteVidMode)
			FCEU_ResetVidSys();

		PowerNES();

		if (GameInfo->type != GIT_NSF)
			FCEU_LoadGamePalette();

		FCEU_ResetPalette();

		if (!lastpal && PAL)
		{
			FCEU_DispMessage("PAL mode set");
			FCEUI_printf("PAL mode set");
		}
		else if (!lastdendy && dendy)
		{
			// this won't happen, since we don't autodetect dendy, but maybe someday we will?
			FCEU_DispMessage("Dendy mode set");
			FCEUI_printf("Dendy mode set");
		}
		else if ((lastpal || lastdendy) && !(PAL || dendy))
		{
			FCEU_DispMessage("NTSC mode set");
			FCEUI_printf("NTSC mode set");
		}
	}
	else
	{
		if (!silent)
			FCEU_PrintError("An error occurred while loading the file.");

		free(GameInfo);
		GameInfo = 0;
	}

	FCEU_fclose(fp);
	return GameInfo;
}

FCEUGI *
FCEUI_LoadGame(const char *name, int OverwriteVidMode, bool silent)
{
	return FCEUI_LoadGameVirtual(name, OverwriteVidMode, silent);
}

// Return: Flag that indicates whether the function was succesful or not.
bool FCEUI_Initialize()
{
	srand(0);

	if (!FCEU_InitVirtualVideo())
	{
		return false;
	}

	// Initialize some parts of the settings structure
	// mbg 5/7/08 - I changed the ntsc settings to match pal.
	// this is more for precision emulation, instead of entertainment, which is what fceux is all about nowadays
	memset(&FSettings, 0, sizeof(FSettings));
	// FSettings.UsrFirstSLine[0]=8;
	FSettings.UsrFirstSLine[0] = 0;
	FSettings.UsrFirstSLine[1] = 0;
	// FSettings.UsrLastSLine[0]=231;
	FSettings.UsrLastSLine[0] = 239;
	FSettings.UsrLastSLine[1] = 239;
	FSettings.SoundVolume = 150;	// 0-150 scale
	FSettings.TriangleVolume = 256; // 0-256 scale (256 is max volume)
	FSettings.Square1Volume = 256;	// 0-256 scale (256 is max volume)
	FSettings.Square2Volume = 256;	// 0-256 scale (256 is max volume)
	FSettings.NoiseVolume = 256;	// 0-256 scale (256 is max volume)
	FSettings.PCMVolume = 256;		// 0-256 scale (256 is max volume)

	FCEUPPU_Init();

	X6502_Init();

	return true;
}

void FCEUI_Kill(void)
{
	FCEU_KillVirtualVideo();
	FCEU_KillGenie();
}

/// Emulates a single frame.

/// Skip may be passed in, if FRAMESKIP is #defined, to cause this to emulate more than one frame
void FCEUI_Emulate(uint8 **pXBuf, int32 **SoundBuf, int32 *SoundBufSize, int skip)
{
	// for(int i = 0;i< 256*256;i++) {
	// 	if(XBuf[i] != 128)
	// 	printf("%d ", XBuf[i]);
	// }
	// printf("\n");
	// skip initiates frame skip if 1, or frame skip and sound skip if 2
	int ssize;

	if (frameAdvanceRequested)
	{
		if (frameAdvance_Delay_count == 0 || frameAdvance_Delay_count >= frameAdvance_Delay)
			EmulationPaused = EMULATIONPAUSED_FA;
		if (frameAdvance_Delay_count < frameAdvance_Delay)
			frameAdvance_Delay_count++;
	}

	if (EmulationPaused & EMULATIONPAUSED_FA)
	{
		// the user is holding Frame Advance key
		// clear paused flag temporarily

		EmulationPaused &= ~EMULATIONPAUSED_PAUSED;
	}
	else
	{
		if (EmulationPaused & EMULATIONPAUSED_PAUSED)
		{
			// emulator is paused
			memcpy(XBuf, XBackBuf, 256 * 256);
			*pXBuf = XBuf;
			*SoundBuf = WaveFinal;
			*SoundBufSize = 0;
			return;
		}
	}

	FCEU_UpdateInput();

	FCEUPPU_Loop(skip);

	if (skip != 2)
		ssize = FlushEmulateSound(); // If skip = 2 we are skipping sound processing

	timestampbase += timestamp;
	timestamp = 0;
	soundtimestamp = 0;
	// printf("\nskip : %d\n", skip);
	*pXBuf = skip ? 0 : XBuf;
	if (skip == 2)
	{ // If skip = 2, then bypass sound
		*SoundBuf = 0;
		*SoundBufSize = 0;
	}
	else
	{
		*SoundBuf = WaveFinal;
		*SoundBufSize = ssize;
	}

	if ((EmulationPaused & EMULATIONPAUSED_FA) && (!frameAdvanceLagSkip))
	// Lots of conditions here.  EmulationPaused & EMULATIONPAUSED_FA must be true.  In addition frameAdvanceLagSkip or lagFlag must be false
	//  When Frame Advance is held, emulator is automatically paused after emulating one frame (or several lag frames)
	{
		EmulationPaused = EMULATIONPAUSED_PAUSED; // restore EMULATIONPAUSED_PAUSED flag and clear EMULATIONPAUSED_FA flag
	}
}

void FCEUI_CloseGame(void)
{
	FCEU_CloseGame();
}

void ResetNES(void)
{
	if (!GameInfo)
		return;
	GameInterface(GI_RESETM2);
	FCEUSND_Reset();
	FCEUPPU_Reset();
	X6502_Reset();

	// clear back baffer
	extern uint8 *XBackBuf;
	memset(XBackBuf, 0, 256 * 256);

	FCEU_DispMessage("Reset");
}

u64 xoroshiro128plus_next()
{
	return 0;
}

void FCEU_MemoryRand(uint8 *ptr, uint32 size, bool default_zero)
{
}

void PowerNES(void)
{
	if (!GameInfo)
		return;

	// FCEU_CheatResetRAM();
	// FCEU_CheatAddRAM(2, 0, RAM);

	// FCEU_GeniePower();

	FCEU_MemoryRand(RAM, 0x800);

	SetReadHandler(0x0000, 0xFFFF, ANull);
	SetWriteHandler(0x0000, 0xFFFF, BNull);

	SetReadHandler(0, 0x7FF, ARAML);
	SetWriteHandler(0, 0x7FF, BRAML);

	SetReadHandler(0x800, 0x1FFF, ARAMH);  // Part of a little
	SetWriteHandler(0x800, 0x1FFF, BRAMH); // hack for a small speed boost.

	InitializeInput();
	FCEUSND_Power();
	FCEUPPU_Power();

	// Have the external game hardware "powered" after the internal NES stuff.  Needed for the NSF code and VS System code.
	GameInterface(GI_POWER);

	timestampbase = 0;
	X6502_Power();
	// clear back buffer
	extern uint8 *XBackBuf;
	memset(XBackBuf, 0, 256 * 256);

	FCEU_DispMessage("Power on");
}

void FCEU_ResetVidSys(void)
{
	int w;

	if (GameInfo->vidsys == GIV_NTSC)
		w = 0;
	else if (GameInfo->vidsys == GIV_PAL)
	{
		w = 1;
		dendy = 0;
	}
	else
		w = FSettings.PAL;

	PAL = w ? 1 : 0;

	if (PAL)
		dendy = 0;

	if (newppu)
		overclock_enabled = 0;

	normalscanlines = (dendy ? 290 : 240) + newppu; // use flag as number!
	totalscanlines = normalscanlines + (overclock_enabled ? postrenderscanlines : 0);
	FCEUPPU_SetVideoSystem(w || dendy);
	SetSoundVariables();
}

FCEUS FSettings;

void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall)
{
	FSettings.UsrFirstSLine[0] = ntscf;
	FSettings.UsrLastSLine[0] = ntscl;
	FSettings.UsrFirstSLine[1] = palf;
	FSettings.UsrLastSLine[1] = pall;
	if (PAL || dendy)
	{
		FSettings.FirstSLine = FSettings.UsrFirstSLine[1];
		FSettings.LastSLine = FSettings.UsrLastSLine[1];
	}
	else
	{
		FSettings.FirstSLine = FSettings.UsrFirstSLine[0];
		FSettings.LastSLine = FSettings.UsrLastSLine[0];
	}
}

void FCEUI_SetVidSystem(int a)
{
	FSettings.PAL = a ? 1 : 0;
	if (GameInfo)
	{
		FCEU_ResetVidSys();
		FCEU_ResetPalette();
		FCEUD_VideoChanged();
	}
}

int FCEUI_GetCurrentVidSystem(int *slstart, int *slend)
{
	if (slstart)
		*slstart = FSettings.FirstSLine;
	if (slend)
		*slend = FSettings.LastSLine;
	return (PAL);
}

void FCEUI_SetRegion(int region, int notify)
{
	switch (region)
	{
	case 0: // NTSC
		normalscanlines = 240;
		pal_emulation = 0;
		dendy = 0;
		// until it's fixed on sdl. see issue #740
		break;
	case 1: // PAL
		normalscanlines = 240;
		pal_emulation = 1;
		dendy = 0;
		break;
	case 2: // Dendy
		normalscanlines = 290;
		pal_emulation = 0;
		dendy = 1;
		break;
	}
	normalscanlines += newppu;
	totalscanlines = normalscanlines + (overclock_enabled ? postrenderscanlines : 0);
	FCEUI_SetVidSystem(pal_emulation);
	RefreshThrottleFPS();
}

int32 FCEUI_GetDesiredFPS(void)
{
	if (PAL || dendy)
		return 50;
	else
		return 60;
}

int FCEUI_EmulationPaused(void)
{
	return (EmulationPaused & EMULATIONPAUSED_PAUSED);
}

int FCEU_TextScanlineOffset(int y)
{
	return FSettings.FirstSLine * 256;
}
int FCEU_TextScanlineOffsetFromBottom(int y)
{
	return (FSettings.LastSLine - y) * 256;
}
