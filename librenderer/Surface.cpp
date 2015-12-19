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

#include "SDL.h"

#ifdef __EMSCRIPTEN__
	#include "SDL_image.h"
#else
	#include "stb_image.h"
#endif

using namespace Framework;

namespace Engine2D
{

	void Surface::AssertThread()
	{
		//ScreenSurface::AssertThread();
	}

	void Surface::Create()
	{
		AssertThread();

		surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
		if (surface == nullptr)
			THROW GraphicException("Unable to create surface! SDL Error: ", SDL_GetError());

#ifndef __EMSCRIPTEN__
		texture = SDL_CreateTextureFromSurface(ScreenSurface::sdl_2_0_renderer, surface);
		if (texture == nullptr)
			THROW GraphicException("Unable to create texture of dimension ", width, "x", height, "! SDL Error: ", SDL_GetError());
#endif
	}


#ifdef __EMSCRIPTEN__

	Surface::Surface(std::string path)
	{
		surface = IMG_Load(path.c_str());
		if (surface == nullptr)
			THROW GraphicException("Unable to load image \"", path, "\": ", IMG_GetError());

		width = surface->w;
		height = surface->h;
	}

#else


	Surface::Surface(std::string path)
	{
		AssertThread();

		int channels;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
		if (data == nullptr)
			THROW GraphicException("Unable to load image \"", path, "\"!");

		finally data_releaser([&]() { stbi_image_free(data);  });

		Create();

		MapPixels(
			width * height * 4,
			[&](void* ptr)
		{
			auto pixels = (Uint32*)ptr;
			for (int y = 0, iSrc = 0, iDst = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					Uint32 p = 0;
					p |= ((Uint32)data[iSrc++]) << 16;
					p |= ((Uint32)data[iSrc++]) << 8;
					p |= ((Uint32)data[iSrc++]) << 0;
					p |= ((Uint32)data[iSrc++]) << 24;

					pixels[iDst++] = p;
				}
			}
		});

	}


#endif

	void Surface::Lock()
	{
		AssertThread();

		SDL_LockSurface(surface);
		lockedPixels = surface->pixels;
	}

	void Surface::Unlock()
	{
		AssertThread();

		lockedPixels = nullptr;
		SDL_UnlockSurface(surface);
	}

	void* Surface::GetPixels(int expectedSizeInBytes)
	{
		assert(expectedSizeInBytes <= width * height * 4);

		if (lockedPixels == nullptr)
			THROW ApplicationException("Pixels must be locked before you can access them!");

		return lockedPixels;
	}

	bool Surface::MapPixels(int expectedSizeInBytes, std::function<void(void*)> interlockedCallback)
	{
		static int i = 0;

		AssertThread();

		int expected = GetWidth() * GetHeight() * 4;
		if (expected < expectedSizeInBytes)
			return false;

		if (SDL_LockSurface(surface) != 0)
			return false;

		{
			finally surface_unlock([=](){ SDL_UnlockSurface(surface); });

#ifndef __EMSCRIPTEN__
			pixelTemp.resize(expected);
			interlockedCallback(pixelTemp.data());
			unsigned char* swapped = (unsigned char*)surface->pixels;

			for(int i = 0; i < expected; i+= 4) {
				swapped[i+2] = pixelTemp[i+0];
				swapped[i+1] = pixelTemp[i+1];
				swapped[i+0] = pixelTemp[i+2];
				swapped[i+3] = 255; // ignore alpha
			}
#else
			interlockedCallback(surface->pixels);
#endif
		}

#ifndef __EMSCRIPTEN__
		if (SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch) != 0)
			return false;
#endif

		return true;
	}

	Surface::Surface(int width, int height) : width(width), height(height)
	{
		Create();
	}

	Surface::Surface(int width, int height, const std::vector<unsigned char>& data) : width(width), height(height)
	{
		Create();

		if (width * height * 4 > data.size())
			THROW ArgumentException("Given buffer is not large enough to provide all the data.");

		MapPixels(
			width * height * 4,
			[&](void* ptr)
		{
			auto pixels = (unsigned char*)ptr;
			for (int y = 0, iSrc = 0, iDst = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					auto r = data[iSrc++];
					auto g = data[iSrc++];
					auto b = data[iSrc++];

					pixels[iDst++] = b;
					pixels[iDst++] = g;
					pixels[iDst++] = r;
					pixels[iDst++] = data[iSrc++];
				}
			}
		});
	}


	int Surface::GetWidth() { return width; }
	int Surface::GetHeight() { return height; }

	Surface::~Surface()
	{
		AssertThread();

		try
		{
#ifndef __EMSCRIPTEN__
			if (texture != nullptr)
				SDL_DestroyTexture(texture);
#endif

			if (surface != nullptr)
				SDL_FreeSurface(surface);
		}
		catch (...)
		{
		}

		texture = nullptr;
		surface = nullptr;
	}
}
