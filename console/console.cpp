#include "console.hpp"

const HANDLE console::handle{ GetStdHandle(STD_OUTPUT_HANDLE) };
const HANDLE console::handle_input{ GetStdHandle(STD_INPUT_HANDLE) };
//
// c_color
//
void console::c_color::set(_In_ t_color color) noexcept {
	SetConsoleTextAttribute(console::handle, static_cast <int> (color));
}
void console::c_color::set(_In_ b_color color) noexcept {
	SetConsoleTextAttribute(console::handle, static_cast <int> (color));
}
void console::c_color::set(_In_ int color) noexcept {
	SetConsoleTextAttribute(console::handle, static_cast <int> (color));
}

void console::c_color::operator= (_In_ t_color color) noexcept {
	this->set(color);
}
void console::c_color::operator= (_In_ b_color color) noexcept {
	this->set(color);
}
void console::c_color::operator= (_In_ int color) noexcept {
	this->set(color);
}

console::c_color console::color{};
console::c_cursor console::cursor{};
console::c_cursor::c_cursor_pos console::c_cursor::pos;
console::c_cursor::c_cursor_info console::c_cursor::info;

//
// c_cursor
//
// c_cursor_pos
//
void console::c_cursor::c_cursor_pos::set(_In_ pos_t _value) noexcept {
	SetConsoleCursorPosition(console::handle, _value);
}

void console::c_cursor::c_cursor_pos::operator= (_In_ pos_t _value) noexcept {
	this->set(_value);
}

//
// c_cursor
//
// c_cursor_info
//
void console::c_cursor::c_cursor_info::set(_In_ info_t _value) noexcept {
	SetConsoleCursorInfo(console::handle, &_value);
}

void console::c_cursor::c_cursor_info::operator= (_In_ info_t _value) noexcept {
	this->set(_value);
}