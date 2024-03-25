#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

class console {
public:
	enum class t_color;
	enum class b_color;

	using pos_t = COORD;
	using info_t = CONSOLE_CURSOR_INFO;
private:
	class c_color {
	public:
		void set(_In_ t_color color) noexcept;
		void set(_In_ b_color color) noexcept;
		void set(_In_ int color) noexcept;

		void operator= (_In_ t_color color) noexcept;
		void operator= (_In_ b_color color) noexcept;
		void operator= (_In_ int color) noexcept;

		c_color(void) noexcept {}
		~c_color(void) noexcept {}
	};
	
	class c_cursor {
	private:
		class c_cursor_pos {
		public:
			void set(_In_ pos_t value) noexcept;

			void operator= (_In_ pos_t value) noexcept;

			c_cursor_pos(void) noexcept {}
			~c_cursor_pos(void) noexcept {}
		};

		class c_cursor_info {
		public:
			void set(_In_ info_t value) noexcept;

			void operator= (_In_ info_t value) noexcept;

			c_cursor_info(void) noexcept {}
			~c_cursor_info(void) noexcept {}
		};
	public:

		static c_cursor_pos pos;
		static c_cursor_info info;

		c_cursor(void) noexcept {}
		~c_cursor(void) noexcept {}
	};
public:
	const static HANDLE handle;
	const static HANDLE handle_input;

	static c_color color;
	static c_cursor cursor;
	
	enum class t_color {
		BLACK = 0,
		DARKBLUE = FOREGROUND_BLUE,
		DARKGREEN = FOREGROUND_GREEN,
		DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
		DARKRED = FOREGROUND_RED,
		DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
		DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
		DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		GRAY = FOREGROUND_INTENSITY,
		BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
		MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	};

	enum class b_color {
		BLACK = 0,
		DARKBLUE = BACKGROUND_BLUE,
		DARKGREEN = BACKGROUND_GREEN,
		DARKCYAN = BACKGROUND_GREEN | BACKGROUND_BLUE,
		DARKRED = BACKGROUND_RED,
		DARKMAGENTA = BACKGROUND_RED | BACKGROUND_BLUE,
		DARKYELLOW = BACKGROUND_RED | BACKGROUND_GREEN,
		DARKGRAY = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
		GRAY = BACKGROUND_INTENSITY,
		BLUE = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
		GREEN = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
		CYAN = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
		RED = BACKGROUND_INTENSITY | BACKGROUND_RED,
		MAGENTA = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
		YELLOW = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
		WHITE = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
	};
};