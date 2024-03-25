#pragma once

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <filesystem>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")