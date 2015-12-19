#include "S9xBridge.hpp"

using namespace SNESOnline;


extern "C" int gameconsole_get_screen_width() { return S9xBridge::screenWidth; }
extern "C" int gameconsole_get_screen_height() { return S9xBridge::screenHeight; }

	extern "C" bool gameconsole_read_screen(int width, int height, int* pixelsRgbX)
	{
		if((width < 0) || (height < 0) || (width * height * 4 < S9xBridge::pixels.size())) {
			return false;
		}

		memcpy(pixelsRgbX, S9xBridge::pixels.data(), S9xBridge::pixels.size());
		return true;
	}

	extern "C" bool gameconsole_reset(const char* romFile, const char* sramFile)
	{
		return S9xBridge::Startup(romFile, sramFile);
	}

	extern "C" void gameconsole_read_audio(uint64_t audioTime, int16_t* pcmData, int pcmDataSizeInBytes)
	{
		S9xBridge::OnFillAudioBuffer(audioTime, pcmData, pcmDataSizeInBytes);
	}
