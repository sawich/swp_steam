#pragma once

#include "headers.hpp"

namespace ea {
	class window {
	public:
		HWND hwnd (void) const noexcept { return m_hwnd; }
		HINSTANCE hinstance (void) const noexcept { return m_hinstance; }

		template <typename F>
		void loop (F && _func) noexcept {
			do {
				while (PeekMessage (&m_msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage (&m_msg);
					DispatchMessage (&m_msg);
				}
			} while (WM_QUIT != m_msg.message);
		}

		window& operator= (window && _window) noexcept {
			m_hwnd = std::move (_window.m_hwnd);
			m_hinstance = std::move (_window.m_hinstance);

			_window.m_hwnd = NULL;
			_window.m_hinstance = NULL;

			return *this;
		}

		window (window && _window) noexcept :
			m_hwnd{ _window.m_hwnd },
			m_hinstance{ _window.m_hinstance }
		{
			_window.m_hwnd = NULL;
			_window.m_hinstance = NULL;
		}

		window (void) noexcept :
			m_hwnd{ NULL },
			m_hinstance{ NULL }
		{}

		window (HWND && _hwnd, HINSTANCE && _hinstance = GetModuleHandle (nullptr)) noexcept :
			m_hwnd{ std::move (_hwnd) },
			m_hinstance{ std::move (_hinstance) }
		{}

		window (
			const int _width,
			const int _height,
			const TCHAR * _app_name,
			WNDPROC _proc,
			const HINSTANCE _hInstance = GetModuleHandle (nullptr)
		) noexcept : m_hinstance{ _hInstance } {
			WNDCLASSEX wcex{};
			wcex.cbSize = sizeof (wcex);
			wcex.lpfnWndProc = _proc;
			wcex.hInstance = m_hinstance;
			wcex.lpszClassName = _app_name;
			wcex.hIcon = LoadIcon (m_hinstance, IDI_APPLICATION);
			wcex.hIconSm = LoadIcon (m_hinstance, IDI_APPLICATION);
			wcex.hCursor = LoadCursor (NULL, IDC_ARROW);

			if (!RegisterClassEx (&wcex)) {
				message_error_and_exit ("RegisterClassEx");
			}

			RECT rect = { 0, 0, static_cast <LONG> (_width), static_cast <LONG> (_height) };
			if (!AdjustWindowRectEx (&rect, WS_EX_OVERLAPPEDWINDOW, false, 0)) {
				message_error_and_exit ("AdjustWindowRectEx");
			}

			m_hwnd = CreateWindowEx (
				NULL, _app_name, _app_name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
				rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, m_hinstance, NULL);

			if (!m_hwnd) {
				message_error_and_exit ("CreateWindowEx");
			}
		}

		~window (void) noexcept {
			if (NULL != m_hwnd) {
				DestroyWindow (m_hwnd);
			}
		}
	private:
		MSG m_msg{};
		HWND m_hwnd{ NULL };
		HINSTANCE m_hinstance;
	};
}