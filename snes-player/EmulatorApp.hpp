
#pragma once

#include "Engine2D.hpp"

namespace SNESOnline
{
	class EmulatorApp : public Engine2D::Application
	{
	private:
		static Engine2D::AppSettings GetAppSettings();

		std::shared_ptr<Engine2D::Surface> renderTarget;
		std::vector<unsigned char> pixels;

	protected:

		void OnShutdown() override;
		void OnRender() override;
		void OnFixedUpdate() override;
		void OnStartup() override;
		void OnFillAudioBuffer(uint64_t audioTime, std::vector<int16_t>& pcmData) override;
	public:

		EmulatorApp();
		~EmulatorApp();
	};
}
