
#pragma once

#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <array>

#include <string.h>

namespace SNES {
			void ShutdownSnes9X();
			void OnFillAudioBuffer(uint64_t audioTime, int16_t* pcmData, int pcmDataSizeInBytes);
			bool StartupSnes9X(std::string romFile, std::string sramFile);

		enum class S9xGamepadButtons
		{
			X, B, A, R, L, Y, Start, Select,
			Left, Up, Down, Right,
		};

		void S9xSetGamepadState(int gamePadId, std::vector<S9xGamepadButtons> pressedButtons);


}

namespace SNESOnline
{
	enum class LogLevel
	{
		Info,
		Debug,
		Warning,
		Error,
		Fatal,
	};


	class S9xBridge
	{
	public:
		static int MouseX;
		static int MouseY;
		static std::vector<unsigned char> pixels;
		static int screenWidth;
		static int screenHeight;
#ifndef __EMSCRIPTEN__
		static std::mutex mutex;
#endif

		static void Log(LogLevel level, std::string message);

		static void DoExit() {}
		static void OnFillAudioBuffer(uint64_t audioTime, int16_t* pcmData, int pcmDataSizeInBytes);
		static void SaveState(std::string fileName, const std::string& state);
		static void Shutdown();
		static bool Startup(std::string romFile, std::string sramFile);
		static void SetGamepadState(int gamePadId, std::vector<SNES::S9xGamepadButtons> pressedButtons);
	};
}
