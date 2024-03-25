#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "../console/console.hpp"
#include "md5.hpp"

#include <vector>
#include <array>
#include <execution>
#include <codecvt>
#include <clocale>
#include <filesystem>
#include <regex>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <map>

#include "../libs/ziplib/Source/ZipLib/ZipFile.h"
#include "../libs/ziplib/Source/ZipLib/streams/memstream.h"
#include "../libs/ziplib/Source/ZipLib/utils/stream_utils.h"
#include "../libs/ziplib/Source/ZipLib/methods/DeflateMethod.h"

#pragma warning( push )
#pragma warning( disable : 4307)