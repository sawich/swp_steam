#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "../swp_steam_lib/header.hpp"

#include <windows.h>
#include <wininet.h>
#include <shobjidl.h> 
#include <filesystem>
#include <future>
#include <sstream>
#include <future>

#pragma comment(lib, "Wininet.lib")


#define WIN_DBG(func, ...) \
	if (FAILED (func (##__VA_ARGS__))) {\
		MessageBox (NULL, TEXT (#func), TEXT ("Ошибка"), MB_OK | MB_ICONERROR);\
	}

#define message_error_and_exit(message) MessageBoxA(NULL, message, "error", MB_OK | MB_ICONERROR); std::exit(EXIT_FAILURE)