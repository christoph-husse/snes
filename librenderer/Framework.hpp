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

#pragma once

#ifdef _MSC_VER

	#define _CRT_SECURE_NO_WARNINGS

	#pragma warning (disable: 4100) // unreferenced formal parameter
	#pragma warning (disable: 4505) // unreferenced local function was removed

	#ifdef DEBUG
		#pragma warning (disable: 4189) // unreferenced local variable
		#include <crtdbg.h>
	#endif
#endif

#include <memory.h>
#include <string>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <array>
#include <fstream>
#include <atomic>
#include <mutex>
#include <random>
#include <queue>
#include <thread>
#include <numeric>

#ifdef _WIN32
	#include <io.h>

	#undef min
	#undef max
#else
	#include <unistd.h>
#endif

#include "Atomic.hpp"
#include "NonCopyable.hpp"
#include "Finally.hpp"
#include "Array2D.hpp"
#include "StringUtils.hpp"
#include "Exceptions.hpp"
#include "Base64.hpp"

#include "FileUtils.hpp"
#include "BinaryReader.hpp"
#include "BinaryWriter.hpp"
