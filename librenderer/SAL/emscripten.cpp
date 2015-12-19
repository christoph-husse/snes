/*
The MIT License (MIT)

Copyright (c) 2014 Christoph Husse

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "Engine2D.hpp"

#ifdef __EMSCRIPTEN__

#include "emscripten/emscripten.h"
#include "SDL.h"

namespace Engine2D
{

	ScreenSurface::ScreenSurface(AppSettings settings)
	{
		auto flags = SDL_INIT_VIDEO;

		if (SDL_Init(flags) < 0)
			THROW GraphicException("Unable to Init Engine2D: ", SDL_GetError());

		sdl_1_2_surface = SDL_SetVideoMode(settings.windowWidth, settings.windowHeight, 16, SDL_DOUBLEBUF);
	}

	ScreenSurface::~ScreenSurface()
	{
	}

	void ScreenSurface::Clear(int r, int g, int b)
	{
		SDL_FillRect(sdl_1_2_surface, nullptr, SDL_MapRGBA(sdl_1_2_surface->format, (Uint8)r, (Uint8)g, (Uint8)b, SDL_ALPHA_OPAQUE));
	}

	void ScreenSurface::Present()
	{
		static int lastWidth = 0, lastHeight = 0;

		SDL_UpdateRect(sdl_1_2_surface, 0, 0, 0, 0);

		int width = Environment::GetCanvasWidth();
		int height = Environment::GetCanvasHeight();

		if((lastWidth != width) || (lastHeight != height))
		{
			lastWidth = width;
			lastHeight = height;
			sdl_1_2_surface = SDL_SetVideoMode(width, height, 16, SDL_DOUBLEBUF);
		}
	}

	void ScreenSurface::AssertThread()
	{
	}

	void ScreenSurface::DrawTexture(std::shared_ptr<Surface> surface, fRect src, fRect dst)
	{
		SDL_Rect _src = src, _dst = dst;
		SDL_BlitScaled(surface->surface, &_src, sdl_1_2_surface, &_dst);
	}

	void ScreenSurface::DrawTexture(std::shared_ptr<Surface> surface, fRect dst)
	{
		SDL_Rect _dst = dst;
		SDL_BlitScaled(surface->surface, nullptr, sdl_1_2_surface, &_dst);
	}

	// JS imports
	extern "C"
	{
		int GetCanvasWidth();
		int GetCanvasHeight();
		void SaveState(const char* fileName, const char* state);
	}

	static PcmAudioCallback* pcmPlayback_Handler = nullptr;
	static std::vector<int16_t> pcmPlayback_Buffer(2048);
	static EventObserver* eventObserver = nullptr;
	static std::vector<unsigned char> applicationState;
	static std::string applicationStateBase64;

	extern "C" int pcmPlayback_GetBufferAt(int i)
	{
		return pcmPlayback_Buffer[i];
	}

	extern "C" int pcmPlayback_PrepareBuffer(int len)
	{
		if(pcmPlayback_Handler != nullptr)
		{
			pcmPlayback_Buffer.resize(len);
			pcmPlayback_Handler(pcmPlayback_Buffer);
			return 1;
		}
		else
			return 0;
	}

	extern "C" void input_OnGestureEvent(int gestureType, int touchX, int touchY)
	{
		if(eventObserver == nullptr)
			return;

		GestureEvent gesture;
		gesture.type = (GestureType)gestureType;
		gesture.x = touchX;
		gesture.y = touchY;

		eventObserver->OnGestureEvent(gesture);
	}

	extern "C" void input_OnKeyboardEvent(int knownKey, int physicalId, int symbolic, int isPressed)
	{
		if(eventObserver == nullptr)
			return;

		KeyInfo key;
		key.known = (KnownKey)knownKey;
		key.physical = physicalId;
		key.symbolic = symbolic;

		if(isPressed)
			eventObserver->OnKeyDown(key);
		else
			eventObserver->OnKeyUp(key);
	}

	void PcmPlayback::SetCallback(PcmAudioCallback* handler)
	{
		memset(pcmPlayback_Buffer.data(), 0, pcmPlayback_Buffer.size() * sizeof(pcmPlayback_Buffer[0]));

		pcmPlayback_Handler = handler;
	}

	void Environment::SaveState(std::string fileName, std::string state)
	{
		::Engine2D::SaveState(fileName.c_str(), state.c_str());
	}

	void EventMapper::SetEventObserver(EventObserver* observer)
	{
		eventObserver = observer;
	}

	int Environment::GetCanvasWidth()
	{
		return Engine2D::GetCanvasWidth();
	}

	int Environment::GetCanvasHeight()
	{
		return Engine2D::GetCanvasHeight();
	}
}

#endif
