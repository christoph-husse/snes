#include "S9xBridge.hpp"

#include <iostream>

namespace SNES {
	extern uint64_t globalSnesTimer;
}
namespace SNESOnline
{

	int S9xBridge::MouseX = 0;
	int S9xBridge::MouseY = 0;
	std::vector<unsigned char> S9xBridge::pixels;
	int S9xBridge::screenWidth = 0;
	int S9xBridge::screenHeight = 0;

#ifndef __EMSCRIPTEN__
	std::mutex S9xBridge::mutex;
#endif

	void S9xBridge::Log(LogLevel level, std::string message)
	{
		switch (level)
		{
		case LogLevel::Debug:
		case LogLevel::Info:
			std::cout << message << std::endl;
		case LogLevel::Fatal:
		case LogLevel::Error:
		case LogLevel::Warning:
			std::cerr << message << std::endl;
		}
	}

	void S9xBridge::SaveState(std::string fileName, const std::string& state)
	{
	}

	void S9xBridge::OnFillAudioBuffer(uint64_t audioTime, int16_t* pcmData, int pcmDataSizeInBytes)
	{
#ifndef __EMSCRIPTEN__
		std::lock_guard<std::mutex> lock(mutex);
#endif

		//while(SNES::globalSnesTimer < 5300) {
		//	::SNES::OnFillAudioBuffer(audioTime, pcmData);
		//}

		::SNES::OnFillAudioBuffer(audioTime, pcmData, pcmDataSizeInBytes);
	}

	void S9xBridge::Shutdown()
	{
#ifndef __EMSCRIPTEN__
		std::lock_guard<std::mutex> lock(mutex);
#endif
		::SNES::ShutdownSnes9X();
	}

	bool S9xBridge::Startup(std::string romFile, std::string sramFile)
	{
#ifndef __EMSCRIPTEN__
		std::lock_guard<std::mutex> lock(mutex);
#endif
		return ::SNES::StartupSnes9X(romFile, sramFile);
	}

	void S9xBridge::SetGamepadState(int gamePadId, std::vector<SNES::S9xGamepadButtons> pressedButtons)
	{
#ifndef __EMSCRIPTEN__
		std::lock_guard<std::mutex> lock(mutex);
#endif
		std::vector<::SNES::S9xGamepadButtons> btns;
		for (auto e : pressedButtons)
			btns.push_back((::SNES::S9xGamepadButtons)e);

		::SNES::S9xSetGamepadState(gamePadId, btns);
	}
}
