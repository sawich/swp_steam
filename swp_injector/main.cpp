#define WIN32_LEAN_AND_MEAN

#include "../console/console.hpp"

#include <string_view>
#include <iostream>
#include <filesystem>

#include <windows.h>
#include <TCHAR.h>

#ifdef _WIN32
#	define _X86_
#else
#	define _AMD64_
#endif
#include "../libs/detours/detours.h"

namespace an {
#ifdef _UNICODE
	using string_view = ::std::wstring_view;
	using string = ::std::wstring;
	inline size_t tcslen(const LPTSTR _str) noexcept { return ::wcslen(_str); };
#else
	using string_view = ::std::string_view;
	using string = ::std::string;
	inline size_t tcslen(const LPTSTR _str) noexcept { return ::strlen(_str); };
#endif

}

#define DBG_MESSAGE(msg) console::color.set(console::t_color::DARKGRAY);std::cout << "[o]   >> "##msg"   "
#define DBG_MESSAGE_OK() console::color.set(console::t_color::GREEN);std::cout << "ok!\n"
#define DBG_MESSAGE_ER() console::color.set(console::t_color::RED);std::cout << "error!\n"

class tools {
	class process_handle {
		HANDLE m_handle;
	public:
		HANDLE& get() noexcept {
			return m_handle;
		}

		bool operator== (const process_handle& _if) const noexcept {
			return m_handle == _if.m_handle;
		}

		bool operator== (const HANDLE& _if) const noexcept {
			return m_handle == _if;
		}

		process_handle(HANDLE&& _handle) noexcept : m_handle{ _handle } {}

		~process_handle(void) noexcept {
			if (INVALID_HANDLE_VALUE == m_handle) {
				return;
			}

			CloseHandle(m_handle);
		}
	};

	class virtual_alloc_ex {
		size_t m_size{ 0 };
		LPVOID m_ptr{ nullptr };
		PROCESS_INFORMATION& m_process;
	public:
		LPVOID get(void) noexcept { return m_ptr; }

		bool operator== (const virtual_alloc_ex& _if) const noexcept {
			return m_ptr == _if.m_ptr;
		}

		bool operator== (const VOID* _if) const noexcept {
			return m_ptr == _if;
		}

		virtual_alloc_ex(PROCESS_INFORMATION& _process, size_t _size, LPVOID _ptr) noexcept :
			m_process{ _process },
			m_size{ _size },
			m_ptr{ _ptr }
		{}
		~virtual_alloc_ex(void) noexcept {
			if (m_ptr) {
				VirtualFreeEx(m_process.hProcess, m_ptr, m_size, MEM_RELEASE);
			}
		}
	};
public:
	static PROCESS_INFORMATION spawn(an::string_view _app_path) noexcept {
		STARTUPINFO startup_info{ 0 };
		PROCESS_INFORMATION process_info{ 0 };

		startup_info.cb = sizeof(startup_info);

		DBG_MESSAGE("spawn process");

		std::string args{ _app_path };
		args += TEXT(" --ip 194.187.19.129");
		args += TEXT(" --port 10000");
		args += TEXT(" --steam");

		
		if (FAILED (CreateProcessA(NULL, const_cast <an::string_view::value_type *> (std::data(args)), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startup_info, &process_info))) {
			DBG_MESSAGE_ER();
			return{ 0 };
		}

		DBG_MESSAGE_OK();

		return{ process_info };
	}

	static void inject(PROCESS_INFORMATION& _process, an::string_view _dll_path) noexcept {
		const size_t memory{ (std::size(_dll_path) + 1) * sizeof(an::string_view::value_type) };
		
		DBG_MESSAGE("VirtualAllocEx");

		virtual_alloc_ex dll_memory(_process, MAX_PATH, VirtualAllocEx(_process.hProcess, NULL, memory, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE));
		if (dll_memory == NULL) {
			DBG_MESSAGE_ER();
			return;
		}

		DBG_MESSAGE_OK();
		DBG_MESSAGE("GetFileAttributes");

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(std::data(_dll_path))) {
			DBG_MESSAGE_ER();
			return;
		}

		DBG_MESSAGE_OK();
		DBG_MESSAGE("WriteProcessMemory");

		if (FALSE == WriteProcessMemory(_process.hProcess, dll_memory.get (), std::data(_dll_path), memory, NULL)) {
			DBG_MESSAGE_ER();
			return;
		}

		DBG_MESSAGE_OK();
		DBG_MESSAGE("CreateRemoteThread");
		
		tools::process_handle thread{ CreateRemoteThread(_process.hProcess, NULL, 0, reinterpret_cast <LPTHREAD_START_ROUTINE> (LoadLibraryA), dll_memory.get (), NULL, NULL) };
		if (thread == NULL) {
			DBG_MESSAGE_ER();
			return;
		}

		DBG_MESSAGE_OK();
		DBG_MESSAGE("WaitForSingleObject");

		if (WAIT_FAILED == WaitForSingleObject(thread.get(), INFINITE)) {
			DBG_MESSAGE_ER();
			return;
		}

		DBG_MESSAGE_OK();
		DBG_MESSAGE("ResumeThread");

		if (-1 == ResumeThread(_process.hThread)) {
			DBG_MESSAGE_ER();
			return;
		}

		DBG_MESSAGE_OK();
	}
};

INT _tmain (int argc, TCHAR* argv[], TCHAR* envp[]) noexcept
{
	if (argc < 2) { return EXIT_FAILURE; }

	STARTUPINFO startup_info{ 0 };
	PROCESS_INFORMATION process_info{ 0 };

	startup_info.cb = sizeof(startup_info);

	
	LPCSTR * args_dlls = (LPCSTR*)(argv + 2);
	
	if (!DetourCreateProcessWithDllsA(NULL, argv[1], NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, std::data(std::filesystem::path(argv[1]).parent_path().generic_string() + '/'), &startup_info, &process_info, argc - 2, args_dlls, NULL)) {
		std::cout << "DetourCreateProcessWithDllEx error: " << GetLastError();
	}

	ResumeThread(process_info.hThread);

	WaitForSingleObject(process_info.hProcess, INFINITE);

	return EXIT_SUCCESS;
}