#include "S9xBridge.hpp"

#include "snes9x.h"
#include "gfx.h"
#include "memmap.h"
#include "filter/2xsai.h"
#include "filter/hq2x.h"
#include "apu/apu.h"
#include "cheats.h"
#include "display.h"
#include "conffile.h"

#include <sstream>
#include <algorithm>
#include <array>
#include <unistd.h>

using namespace SNES;

	namespace internal
	{
		inline void strconcat(std::ostringstream& stream)
		{
		}

		template<class Head, class... StringConcat>
		inline void strconcat(std::ostringstream& stream, const Head& head, const StringConcat&... tail)
		{
			stream << head;
			strconcat(stream, tail...);
		}
	}

	// uses a stringstream to concatenate all given arguments into one single string
	template<class... StringConcat>
	std::string strconcat(const StringConcat&... args)
	{
		std::ostringstream msg;
		internal::strconcat(msg, args...);
		return msg.str();
	}


#define EXT_WIDTH (MAX_SNES_WIDTH + 4)
#define EXT_PITCH (EXT_WIDTH * 2)
#define EXT_HEIGHT (MAX_SNES_HEIGHT + 4)
#define EXT_OFFSET (EXT_PITCH * 2 + 2 * 2)

static void assert_always(bool condition) {
	if(!condition) {
		throw std::bad_exception();
	}
}

#ifndef _WIN32
	#define _access access
#endif


	using namespace SNESOnline;

	static void DoRender();

	struct SSurface {
		unsigned char *Surface;

		uint32_t Pitch;
		uint32_t Width, Height;
	};

	SSurface Src = {};
	bool8 do_frame_adjust = false;

	void SetInfoDlgColor(unsigned char r, unsigned char g, unsigned char b)
	{
	}


void S9xParsePortConfig(ConfigFile &, int pass) {

}

	bool S9xPollAxis(uint32 id, int16 *value){
		return false;
	}

	bool S9xPollPointer(uint32 id, int16 *x, int16 *y){

		if (id & 0x00200000)
		{
			*x = S9xBridge::MouseX;
			*y = S9xBridge::MouseY;
		}
		else
			*x = *y = 0;
		return (true);
	}

	bool8 S9xOpenSnapshotFile(const char *fname, bool8 read_only, STREAM *file)
	{
		char filename[_MAX_PATH + 1];
		char drive[_MAX_DRIVE + 1];
		char dir[_MAX_DIR + 1];
		char fn[_MAX_FNAME + 1];
		char ext[_MAX_EXT + 1];

		_splitpath(fname, drive, dir, fn, ext);
		_makepath(filename, drive, dir, fn, ext[0] == '\0' ? ".000" : ext);

		if (read_only)
		{
			if ((*file = OPEN_STREAM(filename, "rb")))
				return (true);
		}
		else
		{
			if ((*file = OPEN_STREAM(filename, "wb")))
				return (true);
			FILE *fs = fopen(filename, "rb");
			if (fs)
			{
				fclose(fs);
				S9xMessage(S9X_ERROR, S9X_FREEZE_FILE_NOT_FOUND, strconcat("Freeze file \"", filename, "\" exists but is read only").c_str());
			}
			else
			{
				S9xMessage(S9X_ERROR, S9X_FREEZE_FILE_NOT_FOUND, strconcat("Cannot create freeze file \"", filename, "\". Directory is read-only or does not exist.").c_str());
			}
		}
		return false;
	}

	const char *S9xBasename(const char *f)
	{
		const char *p;
		if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
			return (p + 1);

	#ifdef __DJGPP
		if (p = _tcsrchr(f, SLASH_CHAR))
			return (p + 1);
	#endif

		return (f);
	}

	void S9xCloseSnapshotFile(STREAM file)
	{
		CLOSE_STREAM(file);
	}


	//  NYI
	const char *S9xChooseFilename(bool8 read_only)
	{
		return nullptr;
	}

	// NYI
	const char *S9xChooseMovieFilename(bool8 read_only)
	{
		return nullptr;
	}


	const char * S9xStringInput(const char *msg)
	{
		return nullptr;
	}

	void S9xToggleSoundChannel(int c)
	{
		//if (c == 8)
		//	GUI.SoundChannelEnable = 255;
		//else
		//	GUI.SoundChannelEnable ^= 1 << c;

		//S9xSetSoundControl(GUI.SoundChannelEnable);
	}

	// for "frame advance skips non-input frames" feature
	void S9xOnSNESPadRead()
	{
		return;
	}


	bool S9xPollButton(uint32 id, bool *pressed){

		*pressed = false;

		if (id & 0x02000000)	// mouse
		{
			switch (id & 0xFF)
			{
			case 0 /* Left */: break;
			case 1 /* Right */: break;
			}
		}
		else
		if (id & 0x04000000)	// superscope
		{
			switch (id & 0xFF)
			{
			case 0:	break;
			case 2 /* Right */:	break;
			case 3 /* Middle */: break;
			case 4 /* Start/Select*/: break;
			case 1 /* Left */:	break;
			}
		}
		else
		if (id & 0x08000000)	// justifier
		{
			if (id & 0x00000100)
			{
				switch (id & 0xFF)
				{
				case 0:	break;
				case 1 /* Left */: break;
				case 2 /* Right */: break;
				}
			}
			else
			{
				switch (id & 0xFF)
				{
				case 0 /* 2p Start */: break;
				case 1 /* 2p A */: break;
				case 2 /* 2p B */: break;
				}
			}
		}

		return true;
	}


	// do the actual rendering of a frame
	bool8 S9xDeinitUpdate(int Width, int Height)
	{
		Src.Width = Width;
		if (Height%SNES_HEIGHT)
			Src.Height = Height;
		else
		{
			if (Height == SNES_HEIGHT)
				Src.Height = SNES_HEIGHT_EXTENDED;
			else Src.Height = SNES_HEIGHT_EXTENDED << 1;
		}
		Src.Pitch = GFX.Pitch;
		Src.Surface = (unsigned char*)GFX.Screen;


		// Clear some of the old SNES rendered image
		// when the resolution becomes lower in x or y,
		// otherwise the image processors (filters) might access
		// some of the old rendered data at the edges.
		{
			static int LastWidth = 0;
			static int LastHeight = 0;

			if (Width < LastWidth)
			{
				const int hh = std::max(LastHeight, Height);
				for (int i = 0; i < hh; i++)
					memset(GFX.Screen + i * (GFX.Pitch >> 1) + Width * 1, 0, 4);
			}
			if (Height < LastHeight)
			{
				const int ww = std::max(LastWidth, Width);
				for (int i = Height; i < LastHeight; i++)
					memset(GFX.Screen + i * (GFX.Pitch >> 1), 0, ww * 2);

				// also old clear extended height stuff from drawing surface
				if ((int)Src.Height > Height)
				for (int i = Height; i < (int)Src.Height; i++)
					memset(Src.Surface + i * Src.Pitch, 0, Src.Pitch);
			}
			LastWidth = Width;
			LastHeight = Height;
		}

		DoRender();

		return (true);
	}

	void S9xExit(void)
	{
		S9xBridge::DoExit();
	}

	void S9xAutoSaveSRAM()
	{
		// TODO: uncomment
		/*
		auto files = Memory.SaveSRAM();
		if (files.empty())
		{
			std::cout << "[WARNING]: Autosave did not produce any save-files." << std::endl;
			return;
		}

		std::cout << "[INFO]: Autosave did produce the following save-files:" << std::endl;

		std::vector<std::string> base64;
		int size = 0;
		for (auto& e : files)
		{
			std::cout << "    > \"" << e.first << "\": " << e.second.size() << " bytes" << std::endl;
			base64.push_back(" File(" + e.first + ")");
			base64.push_back(Base64Encode(e.second));

			size += e.first.size() + 7 + base64.back().size();
		}

		std::string result;
		result.resize(size);
		int offset = 0;
		for (auto& e : base64)
		{
			assert_always(offset + e.size() <= size);
			std::copy(e.begin(), e.end(), result.begin() + offset);
			offset += e.size();
		}

		assert_always(offset == size);

		S9xBridge::SaveState(files.begin()->first, result);
		*/
	}

	// only necessary for avi recording
	bool8 S9xContinueUpdate(int Width, int Height)
	{
		return true;
	}

	void S9xSetPalette(void)
	{
		return;
	}

	bool8 S9xInitUpdate(void)
	{
		return true;
	}



	const char *S9xGetFilenameInc(const char *e, enum s9x_getdirtype dirtype)
	{
		static char resBuffer[PATH_MAX + 1];
		char dir[_MAX_DIR + 1];
		char drive[_MAX_DRIVE + 1];
		char fname[_MAX_FNAME + 1];
		char ext[_MAX_EXT + 1];
		unsigned int i = 0;
		const char *d;
		std::string filename;

		_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
		d = S9xGetDirectory(dirtype);
		do {
			filename = strconcat(d, "/", fname, i, e);
			i++;
		} while (_access(filename.c_str(), 0) == 0 && i != 0);

		strcpy(resBuffer, filename.c_str());
		return (resBuffer);
	}

	const char* S9xGetDirectoryT(enum s9x_getdirtype dirtype)
	{
		const char* rv = "./";

		switch (dirtype){
		default:
			break;
		case ROMFILENAME_DIR:
			static char filename[PATH_MAX];
			strcpy(filename, Memory.ROMFilename);
			if (!filename[0])
				rv = "./";
			for (int i = strlen(filename); i >= 0; i--){
				if ((filename[i] == '/') || (filename[i] == '\\')){
					filename[i] = '\0';
					break;
				}
			}
			rv = filename;
			break;
		}

		return rv;
	}

	const char *S9xGetDirectory(enum s9x_getdirtype dirtype)
	{
		static char path[PATH_MAX] = { 0 };
		strncpy(path, S9xGetDirectoryT(dirtype), PATH_MAX - 1);
		return path;
	}

	const char *S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
	{
		static char resBuffer[PATH_MAX + 1];

		char dir[_MAX_DIR + 1];
		char drive[_MAX_DRIVE + 1];
		char fname[_MAX_FNAME + 1];
		char ext[_MAX_EXT + 1];
		std::string filename;

		_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
		filename = strconcat(S9xGetDirectory(dirtype), "/", fname, ex);
		strcpy(resBuffer, filename.c_str());
		return resBuffer;
	}

	void WinDisplayStringFromBottom(const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap)
	{
		static int	font_width = 8, font_height = 9;

		if (linesFromBottom <= 0)
			linesFromBottom = 1;

		uint16	*dst = GFX.Screen + (IPPU.RenderedScreenHeight - font_height * linesFromBottom) * GFX.RealPPL + pixelsFromLeft;

		int	len = strlen(string);
		int	max_chars = IPPU.RenderedScreenWidth / (font_width - 1);
		int	char_count = 0;

		for (int i = 0; i < len; i++, char_count++)
		{
			if (char_count >= max_chars || (uint8)string[i] < 32)
			{
				if (!allowWrap)
					break;

				dst += font_height * GFX.RealPPL - (font_width - 1) * max_chars;
				if (dst >= GFX.Screen + IPPU.RenderedScreenHeight * GFX.RealPPL)
					break;

				char_count -= max_chars;
			}

			if ((uint8)string[i] < 32)
				continue;

			S9xDisplayChar(dst, string[i]);
			dst += font_width - 1;
		}
	}

	void S9xMessage(int type, int, const char *str)
	{
		switch (type)
		{
		case S9X_INFO:
			S9xBridge::Log(LogLevel::Info, str);
			break;
		case S9X_WARNING:
			S9xBridge::Log(LogLevel::Warning, str);
			break;
		case S9X_ERROR:
			S9xBridge::Log(LogLevel::Error, str);
			break;
		case S9X_FATAL_ERROR:
			S9xBridge::Log(LogLevel::Fatal, str);
			break;
		default:
			S9xBridge::Log(LogLevel::Debug, str);
			break;
		}
	}

	void S9xSyncSpeed(void)
	{
	}


	void S9xPostRomInit()
	{
		// black out the screen
		for (uint32 y = 0; y < (uint32)IPPU.RenderedScreenHeight; y++)
			memset(GFX.Screen + y * GFX.RealPPL, 0, GFX.RealPPL * 2);
	}

	void S9xSetWinPixelFormat()
	{
		S9xSetRenderPixelFormat(RGB565);

		S9xBlit2xSaIFilterDeinit();
		S9xBlitHQ2xFilterDeinit();
	}
namespace SNES {
	void ShutdownSnes9X()
	{
		Settings.StopEmulation = true;

		Memory.Deinit();
		S9xGraphicsDeinit();
		S9xDeinitAPU();
	}
}
	bool LoadROMPlain(std::string filename)
	{
		if (filename.empty())
			return false;

		if (Memory.LoadROM(filename.c_str()))
		{
			S9xStartCheatSearch(&Cheat);
			return true;
		}

		return false;
	}

	bool LoadROM(std::string romFile, std::string sramfile)
	{
		if (romFile.empty())
			return false;

		if (!Memory.LoadROM(romFile.c_str()))
		{
			S9xBridge::Log(LogLevel::Error, strconcat("Could not open ROM \"", romFile, "\"."));
			return false;
		}

		S9xStartCheatSearch(&Cheat);

		Settings.StopEmulation = false;

		if (!sramfile.empty())
			Memory.LoadSRAM(sramfile.c_str());

		Settings.Paused = false;

		return true;
	}

	static std::vector<int16_t> soundStream;

namespace SNES {
	uint64_t globalSnesTimer = 0;

	void OnFillAudioBuffer(::uint64_t audioTime, int16_t* pcmData, int pcmDataSizeInBytes)
	{
		static bool wasEmpty = true, waspressed = false;

		if (Settings.StopEmulation)
			return;

		while (soundStream.size() < (pcmDataSizeInBytes / 2))
		{
			globalSnesTimer++;
			S9xMainLoop();
		}

		// take samples form our audio buffer
		std::copy_n(soundStream.begin(), pcmDataSizeInBytes / 2, pcmData);
		soundStream.erase(soundStream.begin(), soundStream.begin() + (pcmDataSizeInBytes / 2));
	}
}
	void S9xSoundCallback(void *data)
	{
		static std::vector<int16_t> soundBuffer;

		S9xFinalizeSamples();
		int availSamples = S9xGetSampleCount();
		soundBuffer.resize(availSamples);

		if (S9xMixSamples((unsigned char*)soundBuffer.data(), soundBuffer.size()))
		{
			for (int i = 0; i < soundBuffer.size(); i++)
				soundStream.push_back(soundBuffer[i]);

			if (soundStream.size() > 44100 * 2 * 10)
				soundStream.clear(); // something is wrong with our audio thread, so we just forget those samples to not allocate endless memory...
		}
	}

	static std::array<std::vector<S9xGamepadButtons>, 2> pressedButtons;

	void S9xControlsReset(void)
	{
	}

	void S9xControlsSoftReset(void)
	{

	}

	void S9xSetJoypadLatch(bool latch)
	{

	}

	uint8 S9xReadJOYSERn(int n)
	{
		return 0;
	}

namespace SNES {
	void S9xSetGamepadState(int gamePadId, std::vector<S9xGamepadButtons> pressedButtons_)
	{
		assert_always((gamePadId >= 0) && (gamePadId <= 1));
		pressedButtons.at(gamePadId) = pressedButtons_;
	}
}

	void S9xDoAutoJoypad(void)
	{
		uint16_t buttonMask;

		for (int i = 0; i < 2; i++)
		{
			buttonMask = 0;

			for (auto e : pressedButtons[i])
			{
				int16_t native = 0;
				switch (e)
				{
				case S9xGamepadButtons::A: native = SNES_A_MASK; break;
				case S9xGamepadButtons::B: native = SNES_B_MASK; break;
				case S9xGamepadButtons::X: native = SNES_X_MASK; break;
				case S9xGamepadButtons::Y: native = SNES_Y_MASK; break;
				case S9xGamepadButtons::L: native = SNES_TL_MASK; break;
				case S9xGamepadButtons::R: native = SNES_TR_MASK; break;
				case S9xGamepadButtons::Start: native = SNES_START_MASK; break;
				case S9xGamepadButtons::Select: native = SNES_SELECT_MASK; break;
				case S9xGamepadButtons::Left: native = SNES_LEFT_MASK; break;
				case S9xGamepadButtons::Right: native = SNES_RIGHT_MASK; break;
				case S9xGamepadButtons::Up: native = SNES_UP_MASK; break;
				case S9xGamepadButtons::Down: native = SNES_DOWN_MASK; break;
				}
				buttonMask |= native;
			}

			WRITE_WORD(Memory.FillRAM + 0x4218 + i * 2, buttonMask);
			WRITE_WORD(Memory.FillRAM + 0x421c + i * 2, 0);
		}
	}

	bool8 S9xOpenSoundDevice()
	{
		S9xSetSamplesAvailableCallback(S9xSoundCallback, nullptr);
		return true;
	}


	static void DoRender()
	{
		uint16 *lpSrc = reinterpret_cast<uint16 *>(Src.Surface);
		const unsigned int srcPitch = Src.Pitch >> 1;
		S9xBridge::screenWidth = Src.Width;
		S9xBridge::screenHeight = Src.Height;

		S9xBridge::pixels.resize(Src.Width * Src.Height * 4);
		unsigned char* dstPixels = S9xBridge::pixels.data();

		for (int y = 0, iDst = 0, iSrc = 0; y < Src.Height; y++, lpSrc += srcPitch)
		{
			for (int x = 0; x < Src.Width; x++)
			{
				uint16 rgb16 = *(lpSrc + x);

				dstPixels[iDst++] = ((((rgb16) >> 11)) << /*RedShift+3*/ 3);
				dstPixels[iDst++] = ((((rgb16) >> 6) & 0x1f) << /*GreenShift+3*/ 3);
				dstPixels[iDst++] = (((rgb16)& 0x1f) << /*BlueShift+3*/ 3);
				dstPixels[iDst++] = 255;
			}
		}
	}
namespace SNES {
	bool StartupSnes9X(std::string romFile, std::string sramFile)
	{
		static std::vector<unsigned char> screenBuf;

		memset(&Settings, 0, sizeof(Settings));

		Settings.MouseMaster = true;
		Settings.SuperScopeMaster = true;
		Settings.JustifierMaster = true;
		Settings.MultiPlayer5Master = true;
		Settings.FrameTimePAL = 20000;
		Settings.FrameTimeNTSC = 16667;
		Settings.SixteenBitSound = true;
		Settings.Stereo = true;
		Settings.SoundInputRate = 32000;
		Settings.SupportHiRes = true;
		Settings.Transparency = true;
		Settings.AutoDisplayMessages = true;
		Settings.InitialInfoStringTimeout = 120;
		Settings.HDMATimingHack = 100;
		Settings.BlockInvalidVRAMAccessMaster = true;

		Settings.StopEmulation = true;

		Memory.Init();
		Memory.PostRomInitFunc = S9xPostRomInit;

		screenBuf.resize(EXT_PITCH * EXT_HEIGHT);

		GFX.Pitch = EXT_PITCH;
		GFX.RealPPL = EXT_PITCH;
		GFX.Screen = (uint16*)(screenBuf.data() + EXT_OFFSET);

		S9xInitAPU();
		S9xSetWinPixelFormat();
		S9xInitUpdate();
		S9xGraphicsInit();
		S9xSetSoundMute(false);

		Settings.SoundPlaybackRate = 44100;
		Settings.Mute = false;

		S9xInitSound(33, 0);

		Settings.StopEmulation = !LoadROM(romFile, sramFile);
		return !Settings.StopEmulation;
	}
}
