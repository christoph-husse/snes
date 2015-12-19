#include "EmulatorApp.hpp"

using namespace Engine2D;
using namespace Framework;


extern "C" void gameconsole_read_screen(int width, int height, int* picelsRgbX);
extern "C" bool gameconsole_reset(const char* romFile, const char* sramFile);
extern "C" void gameconsole_read_audio(uint64_t audioTime, int16_t* pcmData, int pcmDataSizeInBytes);
extern "C" int gameconsole_get_screen_width();
extern "C" int gameconsole_get_screen_height();

namespace SNESOnline
{
	EmulatorApp::EmulatorApp() : Application(GetAppSettings())
	{

	}

	EmulatorApp::~EmulatorApp()
	{

	}

	Engine2D::AppSettings EmulatorApp::GetAppSettings()
	{
		auto settings = Engine2D::AppSettings();
		settings.fixedFramesPerSecond = 70;
		return settings;
	}

	void EmulatorApp::OnShutdown()
	{
	}

	void EmulatorApp::OnFixedUpdate()
	{
	}

	void EmulatorApp::OnRender()
	{
		int width = gameconsole_get_screen_width();
		int height = gameconsole_get_screen_height();

		if ((width == 0) || (height == 0))
		{
			renderTarget = nullptr;
			return;
		}

		if ((renderTarget == nullptr) || (height != renderTarget->GetHeight()) || (width != renderTarget->GetWidth())){
			pixels.resize(width * height * 4);
			renderTarget = std::make_shared<Engine2D::Surface>(width, height);
		}

		std::cout << "Render. " <<  pixels[10000] << std::endl;

		gameconsole_read_screen(width, height, (int*)pixels.data());

		renderTarget->MapPixels(pixels.size(), [&](void* target)
		{
			memcpy(target, pixels.data(), pixels.size());
		});

			SetLogicalViewport(renderTarget->GetWidth(), renderTarget->GetHeight());
			DrawTexture(renderTarget, { 0, 0, renderTarget->GetWidth(), renderTarget->GetHeight() });
	}

	void EmulatorApp::OnStartup()
	{
		std::string romFile, sramFile;

		for (auto& arg : GetEnvironmentArgs())
		{
			if (arg.find("ROM:") == 0)
			{
				// filename of the ROM to load
				romFile = arg.substr(4);
				std::cout << "Loading ROM from file \"" + romFile + "\"." << std::endl;
			}
			else if (arg.find("SRAM:") == 0)
			{
				// filename of the SRAM to load
				sramFile = arg.substr(5);
				std::cout << "Loading SRAM from file \"" + sramFile + "\"." << std::endl;
			}
		}

		if (!gameconsole_reset(romFile.c_str(), sramFile.c_str()))
		{
			std::cerr << "[FATAL-ERROR]: Could not load ROM file '" << romFile << "'." << std::endl;
		}
	}

	void EmulatorApp::OnFillAudioBuffer(uint64_t audioTime, std::vector<int16_t>& pcmData)
	{
		for(uint64_t i = 0; i < pcmData.size() - 1; i += 2) {
			pcmData[i] = pcmData[i+1] = std::sin(((double)(i + audioTime)) / 30.0 ) * 16000;
		}

		gameconsole_read_audio(audioTime, pcmData.data(), pcmData.size() * 2);
	}

}
