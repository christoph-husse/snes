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

#pragma once

#include "Engine2D.hpp"

struct SDL_Window;
struct SDL_Rect;
struct SDL_Renderer;

namespace Engine2D
{
	class ScreenSurface final
	{
	private:
		friend class Environment;
		friend class Surface;

		SDL_Surface* sdl_1_2_surface = nullptr;
		static SDL_Window* sdl_2_0_surface;
		static SDL_Renderer* sdl_2_0_renderer;

	public:

		void Clear(int r, int g, int b);
		void Present();
		void DrawTexture(std::shared_ptr<Surface> texture, fRect src, fRect dst);
		void DrawTexture(std::shared_ptr<Surface> texture, fRect dst);
		static void AssertThread();

		ScreenSurface(AppSettings settings);

		~ScreenSurface();
	};

	enum class GestureType
	{
		Unknown = 0,
		Touch = 1,
		Release = 2,
		Drag = 3,
	};

	class GestureEvent final
	{
	private:

	public:
		GestureType type;
		int x, y, fingerId;
	};

	enum class KnownKey
	{
		Unknown, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Space, Enter, Tab, Esc, Backspace, Shift, Ctrl, Alt, CapsLock, NumLock,
		Left, Right, Up, Down, Insert, Delete, Home, End, PageUp, PageDown, 
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		N1, N2, N3, N4, N5, N6, N7, N8, N9, N0,
	};

	class KeyInfo
	{
	public:
		int physical;
		int symbolic;
		KnownKey known;
	};

	class Touch
	{
	public:
		fCoord origin;
		fCoord current;

		void Start(fCoord coords);
		void Release(fCoord coords);
		void Drag(fCoord coords);
	};

	class EventObserver
	{
	private:
		std::vector<std::function<void(const GestureEvent&)>> handlers_OnGestureEvent;
		std::vector<std::function<void()>> handlers_OnQuit;
		std::vector<std::function<void(KeyInfo)>> handlers_OnKeyDown;
		std::vector<std::function<void(KeyInfo)>> handlers_OnKeyUp;
		std::function<void(std::vector<unsigned char>&)> handler_OnSaveState;
	public:
		virtual ~EventObserver() { }

		void SetOnStateState(std::function<void(std::vector<unsigned char>&)> handler) { handler_OnSaveState = handler; }
		void RegisterOnQuit(std::function<void()> handler) { handlers_OnQuit.push_back(handler); }
		void RegisterOnGestureEvent(std::function<void(const GestureEvent&)> handler) { handlers_OnGestureEvent.push_back(handler); }
		void RegisterOnKeyDown(std::function<void(KeyInfo)> handler) { handlers_OnKeyDown.push_back(handler); }
		void RegisterOnKeyUp(std::function<void(KeyInfo)> handler) { handlers_OnKeyUp.push_back(handler); }

		void OnSaveState(std::vector<unsigned char>& buffer) { if (handler_OnSaveState != nullptr) handler_OnSaveState(buffer); }
		void OnQuit() { for (auto& handler : handlers_OnQuit) handler(); }
		void OnGestureEvent(const GestureEvent& gesture) { for (auto& handler : handlers_OnGestureEvent) handler(gesture); }
		void OnKeyDown(KeyInfo key) { for (auto& handler : handlers_OnKeyDown) handler(key); }
		void OnKeyUp(KeyInfo key) { for (auto& handler : handlers_OnKeyUp) handler(key); }
	};

	class EventMapper final
	{
	public:
		static void SetEventObserver(EventObserver* observer);
	};

	class Environment final
	{
	public:
		static int GetCanvasWidth();
		static int GetCanvasHeight();
		static void SaveState(std::string fileName, std::string state);
	};

	typedef void PcmAudioCallback(std::vector<int16_t>&);

	class PcmPlayback final
	{
	private:
		friend class ScreenSurface;

		static PcmAudioCallback* sdl_AudioCallback;
		static void FillAudioBuffer(void* udata, uint8_t* stream, int len);
	public:

		static void SetCallback(PcmAudioCallback* handler);
	};
}
