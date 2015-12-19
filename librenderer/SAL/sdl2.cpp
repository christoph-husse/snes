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

#ifndef __EMSCRIPTEN__

#include <SDL.h>

#undef min
#undef max

namespace Engine2D
{
	SDL_Renderer* ScreenSurface::sdl_2_0_renderer = nullptr;
	SDL_Window* ScreenSurface::sdl_2_0_surface = nullptr;
	PcmAudioCallback* PcmPlayback::sdl_AudioCallback = nullptr;
	static int sdl_2_0_thread_id = 0;

	void PcmPlayback::FillAudioBuffer(void* udata, uint8_t* stream, int len)
	{
		static std::vector<int16_t> buffer(1024 * 2);

		assert((len % sizeof(buffer[0])) == 0);

		buffer.resize(len / sizeof(buffer[0]));

		if (PcmPlayback::sdl_AudioCallback == nullptr)
			memset(stream, 0, len);
		else
		{
			memset(buffer.data(), 0, len);

			PcmPlayback::sdl_AudioCallback(buffer);

			memcpy(stream, buffer.data(), std::min(buffer.size() * sizeof(buffer[0]), (size_t)len));
		}
	}


	void ScreenSurface::AssertThread()
	{
		assert(sdl_2_0_thread_id == SDL_ThreadID());
	}

	ScreenSurface::ScreenSurface(AppSettings settings)
	{
		if (sdl_2_0_surface != nullptr)
			THROW GraphicException("Unable to Init Engine2D: Already initialized!");

		auto flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

		if (SDL_Init(flags) < 0)
			THROW GraphicException("Unable to Init Engine2D: ", SDL_GetError());

		SDL_AudioSpec wanted = {};

		wanted.freq = 44100;
		wanted.format = AUDIO_S16;
		wanted.channels = 2;
		wanted.samples = 1024;
		wanted.callback = PcmPlayback::FillAudioBuffer;
		wanted.userdata = nullptr;

		if (SDL_OpenAudio(&wanted, nullptr) < 0)
			THROW GraphicException("Couldn't open audio: ", SDL_GetError());

		SDL_PauseAudio(0);

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

		sdl_2_0_surface = SDL_CreateWindow(
			settings.windowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			settings.windowWidth, settings.windowHeight,
			SDL_WINDOW_RESIZABLE);

		sdl_2_0_renderer = SDL_CreateRenderer(sdl_2_0_surface, -1, 0);

		sdl_2_0_thread_id = SDL_ThreadID();
	}

	KnownKey SdlToKnowKey(int scan)
	{
		switch (scan)
		{
		case SDL_SCANCODE_A: return KnownKey::A;
		case SDL_SCANCODE_B: return KnownKey::B;
		case SDL_SCANCODE_C: return KnownKey::C;
		case SDL_SCANCODE_D: return KnownKey::D;
		case SDL_SCANCODE_F: return KnownKey::F;
		case SDL_SCANCODE_G: return KnownKey::G;
		case SDL_SCANCODE_H: return KnownKey::H;
		case SDL_SCANCODE_I: return KnownKey::I;
		case SDL_SCANCODE_J: return KnownKey::J;
		case SDL_SCANCODE_K: return KnownKey::K;
		case SDL_SCANCODE_L: return KnownKey::L;
		case SDL_SCANCODE_M: return KnownKey::M;
		case SDL_SCANCODE_N: return KnownKey::N;
		case SDL_SCANCODE_O: return KnownKey::O;
		case SDL_SCANCODE_P: return KnownKey::P;
		case SDL_SCANCODE_Q: return KnownKey::Q;
		case SDL_SCANCODE_R: return KnownKey::R;
		case SDL_SCANCODE_S: return KnownKey::S;
		case SDL_SCANCODE_T: return KnownKey::T;
		case SDL_SCANCODE_U: return KnownKey::U;
		case SDL_SCANCODE_V: return KnownKey::V;
		case SDL_SCANCODE_W: return KnownKey::W;
		case SDL_SCANCODE_X: return KnownKey::X;
		case SDL_SCANCODE_Y: return KnownKey::Y;
		case SDL_SCANCODE_Z: return KnownKey::Z;

		case SDL_SCANCODE_1: return KnownKey::N1;
		case SDL_SCANCODE_2: return KnownKey::N2;
		case SDL_SCANCODE_3: return KnownKey::N3;
		case SDL_SCANCODE_4: return KnownKey::N4;
		case SDL_SCANCODE_5: return KnownKey::N5;
		case SDL_SCANCODE_6: return KnownKey::N6;
		case SDL_SCANCODE_7: return KnownKey::N7;
		case SDL_SCANCODE_8: return KnownKey::N8;
		case SDL_SCANCODE_9: return KnownKey::N9;
		case SDL_SCANCODE_0: return KnownKey::N0;

		case SDL_SCANCODE_RETURN: return KnownKey::Enter;
		case SDL_SCANCODE_ESCAPE: return KnownKey::Esc;
		case SDL_SCANCODE_BACKSPACE: return KnownKey::Backspace;
		case SDL_SCANCODE_TAB: return KnownKey::Tab;
		case SDL_SCANCODE_SPACE: return KnownKey::Space;

		case SDL_SCANCODE_CAPSLOCK: return KnownKey::CapsLock;

		case SDL_SCANCODE_F1: return KnownKey::F1;
		case SDL_SCANCODE_F2: return KnownKey::F2;
		case SDL_SCANCODE_F3: return KnownKey::F3;
		case SDL_SCANCODE_F4: return KnownKey::F4;
		case SDL_SCANCODE_F5: return KnownKey::F5;
		case SDL_SCANCODE_F6: return KnownKey::F6;
		case SDL_SCANCODE_F7: return KnownKey::F7;
		case SDL_SCANCODE_F8: return KnownKey::F8;
		case SDL_SCANCODE_F9: return KnownKey::F9;
		case SDL_SCANCODE_F10: return KnownKey::F10;
		case SDL_SCANCODE_F11: return KnownKey::F11;
		case SDL_SCANCODE_F12: return KnownKey::F12;

		case SDL_SCANCODE_HOME: return KnownKey::Home;
		case SDL_SCANCODE_PAGEUP: return KnownKey::PageUp;
		case SDL_SCANCODE_DELETE: return KnownKey::Delete;
		case SDL_SCANCODE_END: return KnownKey::End;
		case SDL_SCANCODE_PAGEDOWN: return KnownKey::PageDown;
		case SDL_SCANCODE_RIGHT: return KnownKey::Right;
		case SDL_SCANCODE_LEFT: return KnownKey::Left;
		case SDL_SCANCODE_DOWN: return KnownKey::Down;
		case SDL_SCANCODE_UP: return KnownKey::Up;

		case SDL_SCANCODE_LCTRL: return KnownKey::Ctrl;
		case SDL_SCANCODE_LSHIFT: return KnownKey::Shift;
		case SDL_SCANCODE_LALT: return KnownKey::Alt;
		case SDL_SCANCODE_RCTRL: return KnownKey::Ctrl;
		case SDL_SCANCODE_RSHIFT: return KnownKey::Shift;
		case SDL_SCANCODE_RALT: return KnownKey::Alt;
		}

		return KnownKey::Unknown;
	}


	static EventObserver* eventObserver = nullptr;

	void ScreenSurface::Present()
	{
		SDL_RenderPresent(sdl_2_0_renderer);

		static int lastMouseButtonPressed = -1;
		SDL_Event event;


		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				eventObserver->OnQuit();
				return;
			}

			GestureEvent gesture;

			switch (event.type)
			{
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				gesture.type = (event.type == SDL_MOUSEBUTTONDOWN) ? GestureType::Touch : GestureType::Release;
				gesture.x = event.button.x;
				gesture.y = event.button.y;
				gesture.fingerId = 10 + event.button.button;

				if (event.type == SDL_MOUSEBUTTONDOWN)
					lastMouseButtonPressed = gesture.fingerId;
				else
					lastMouseButtonPressed = -1;

				eventObserver->OnGestureEvent(gesture);
			}break;
			case SDL_MOUSEMOTION:
			{
				if (lastMouseButtonPressed < 0)
					continue;

				gesture.type = GestureType::Drag;
				gesture.x = event.motion.x;
				gesture.y = event.motion.y;
				gesture.fingerId = lastMouseButtonPressed;

				eventObserver->OnGestureEvent(gesture);
			}break;

			case SDL_KEYDOWN:
			{
				if (event.key.repeat)
					return;

				auto sym = event.key.keysym;
				KeyInfo key;
				key.physical = sym.scancode;
				key.symbolic = sym.sym;
				key.known = SdlToKnowKey(key.physical);
				eventObserver->OnKeyDown(key);
			}break;

			case SDL_KEYUP:
			{
				if (event.key.repeat)
					return;

				auto sym = event.key.keysym;
				KeyInfo key;
				key.physical = sym.scancode;
				key.symbolic = sym.sym;
				key.known = SdlToKnowKey(key.physical);
				eventObserver->OnKeyUp(key);
			}break;

			default:
				continue;
			}
		}
	}

	void ScreenSurface::Clear(int r, int g, int b)
	{
		SDL_SetRenderDrawColor(sdl_2_0_renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)255);
		SDL_RenderClear(sdl_2_0_renderer);
	}

	void ScreenSurface::DrawTexture(std::shared_ptr<Surface> surface, fRect src, fRect dst)
	{
		SDL_Rect _src = src, _dst = dst;
		SDL_RenderCopy(sdl_2_0_renderer, surface->texture, &_src, &_dst);
	}

	void ScreenSurface::DrawTexture(std::shared_ptr<Surface> surface, fRect dst)
	{
		SDL_Rect _dst = dst;
		SDL_RenderCopy(sdl_2_0_renderer, surface->texture, nullptr, &_dst);
	}

	ScreenSurface::~ScreenSurface()
	{
		SDL_CloseAudio();

		if (sdl_2_0_renderer != nullptr)
			SDL_DestroyRenderer(sdl_2_0_renderer);

		if (sdl_2_0_surface != nullptr)
			SDL_DestroyWindow(sdl_2_0_surface);

		sdl_2_0_renderer = nullptr;
		sdl_2_0_surface = nullptr;
		sdl_1_2_surface = nullptr;
	}

	void EventMapper::SetEventObserver(EventObserver* observer)
	{
		eventObserver = observer;
	}

	int Environment::GetCanvasWidth()
	{
		if (ScreenSurface::sdl_2_0_surface == nullptr)
			return 0;

		int result = 0;
		SDL_GetWindowSize(ScreenSurface::sdl_2_0_surface, &result, nullptr);
		return result;
	}

	int Environment::GetCanvasHeight()
	{
		if (ScreenSurface::sdl_2_0_surface == nullptr)
			return 0;

		int result = 0;
		SDL_GetWindowSize(ScreenSurface::sdl_2_0_surface, nullptr, &result);
		return result;
	}

	void Environment::SaveState(std::string fileName, std::string state)
	{
		FileUtils::WriteAllText(fileName, state);
	}

	void PcmPlayback::SetCallback(PcmAudioCallback* handler)
	{
		sdl_AudioCallback = handler;
	}
}

#endif
