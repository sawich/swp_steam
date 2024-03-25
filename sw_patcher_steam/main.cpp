#include "headers.hpp"
#include "ea_window.hpp"

namespace an::winapi 
{
	class PWSTRPtr {
		PWSTR m_ptr;
	public:
		PWSTR get (void) noexcept { return m_ptr; };

		PWSTRPtr (PWSTR _ptr = nullptr) noexcept : m_ptr { _ptr } {}
		PWSTRPtr (void) noexcept {
			if (nullptr != m_ptr) {
				CoTaskMemFree (m_ptr);
			}
		}
	};

	class ShellItemPtr {
		IShellItem * m_ptr;
	public:
		auto get (void) noexcept { return m_ptr; };

		ShellItemPtr (IShellItem * _ptr = nullptr) noexcept : m_ptr{ _ptr } {}
		ShellItemPtr (void) noexcept {
			if (nullptr != m_ptr) {
				m_ptr->Release ();
			}
		}
	};

	class HINTERNETPtr {
		HINTERNET m_ptr;
	public:
		auto & get (void) noexcept { return m_ptr; };

		HINTERNETPtr (HINTERNET _ptr = nullptr) noexcept : m_ptr{ _ptr } {}
		HINTERNETPtr (void) noexcept {
			if (!!m_ptr) {
				InternetCloseHandle (m_ptr);
			}
		}
	};
}

namespace an {
#ifdef _UNICODE
	using tstring = std::wstring;
#define to_tstring std::to_wstring
#else 

#endif
}

class app {
public:
	std::future <std::stringstream> load_file (void) noexcept {
		return std::async (std::launch::async, [&](void) noexcept -> std::stringstream {
			std::stringstream archive{ std::ios::in | std::ios::out | std::ios::binary };
			const auto WEB_URL = TEXT ("https://github.com/AngelDevelopersGroup/soulworker-steam-ru-translate/archive/master.zip");

			DBG_MESSAGE ("make connection");
			an::winapi::HINTERNETPtr WEB_CONNECT{
				InternetOpen (TEXT ("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:54.0) Gecko/20100101 Firefox/54.0"),
					INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0) };

			if (!WEB_CONNECT.get ()) {
				MessageBox (NULL, (std::wstring{ L"Невозможно создать подключение #" } +std::to_wstring (GetLastError ())).c_str (),
					TEXT ("Ошибка"), MB_OK | MB_ICONERROR);
				return archive;
			}
			DBG_MESSAGE_OK ();

			DBG_MESSAGE ("open site");
			an::winapi::HINTERNETPtr WEB_ADDRESS{ InternetOpenUrl (
				WEB_CONNECT.get (), WEB_URL, NULL, 0, 0, 0) };

			if (!WEB_ADDRESS.get ()) {
				MessageBoxW (NULL, (std::wstring{ L"Невозможно установить подключение #" } + std::to_wstring (GetLastError ())).c_str(),
					TEXT ("Ошибка"), MB_OK | MB_ICONERROR);
				return archive;
			}
			DBG_MESSAGE_OK ();

			constexpr DWORD _SIZE{ 307200 };
			DWORD DATA_SIZE{ _SIZE };
			std::unique_ptr <char[]> _DATA_RECIEVED{ std::make_unique <char[]> (DATA_SIZE) };

			DBG_MESSAGE ("download last translation archive");
			do {
				DWORD NO_BYTES_READ{ 0 };
				if (InternetReadFile (WEB_ADDRESS.get (), _DATA_RECIEVED.get (), DATA_SIZE, &NO_BYTES_READ)) {
					if (NO_BYTES_READ == 0) { break; }

					archive << std::string (_DATA_RECIEVED.get (), NO_BYTES_READ);
				} else {
					if (GetLastError () != ERROR_INSUFFICIENT_BUFFER) { break; }

					DATA_SIZE += _SIZE;
					_DATA_RECIEVED = std::make_unique <char[]> (DATA_SIZE);
				}
			} while (true);
			DBG_MESSAGE_OK ();

			return archive;
		});
	}

	std::filesystem::path select_folder () noexcept
	{
		std::filesystem::path path;
		
		WIN_DBG (CoInitializeEx, nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		IFileDialog * file_dialog{ nullptr };
		WIN_DBG (CoCreateInstance, CLSID_FileOpenDialog,
			NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&file_dialog));

		DWORD dwOptions;
		WIN_DBG (file_dialog->GetOptions, &dwOptions);
		WIN_DBG (file_dialog->SetOptions, dwOptions | FOS_PICKFOLDERS);

		WIN_DBG (file_dialog->SetTitle, L"Выберите папку с игрой");
		if (!SUCCEEDED (file_dialog->Show (NULL))) { return{}; }

		IShellItem * t_ShellItemPtr{ nullptr };
		WIN_DBG (file_dialog->GetResult, &t_ShellItemPtr);

		an::winapi::ShellItemPtr item{ t_ShellItemPtr };

		PWSTR t_pszFilePath;
		WIN_DBG (item.get ()->GetDisplayName, SIGDN_FILESYSPATH, &t_pszFilePath);

		an::winapi::PWSTRPtr pszFilePath{ t_pszFilePath };

		path = { std::filesystem::path{ t_pszFilePath } / L"datas" };
		if (!std::filesystem::is_directory (path)) {
			MessageBox (NULL, TEXT ("Игровые архивы не найдены"), TEXT ("Ошибка"), MB_OK | MB_ICONERROR);
			return{};
		}

		CoUninitialize ();

		return path;
	}

	int patch (std::filesystem::path&& _path, std::stringstream&& _archive) noexcept {	
		if (!_archive.rdbuf ()->in_avail ()) { return EXIT_FAILURE; }

		std::filesystem::path path{ _path };

		DBG_MESSAGE ("[game archive] open");
		tools::fs::stream_t <std::ifstream, const std::filesystem::path, std::ios::openmode> 
			original_archive_file (
				std::forward <const std::filesystem::path> (path / "data12.v"),
				std::forward <decltype (std::ios::binary)> (std::ios::binary));

		if (!original_archive_file.is_open()) {
			MessageBox (NULL, TEXT ("Что-то пошло не так"), TEXT ("Ошибка"),
				MB_OK | MB_ICONERROR);
			return EXIT_FAILURE;
		}
		DBG_MESSAGE_OK ();

		auto original_archive_stream{ tools::reader::res_archive (original_archive_file) };
		ZipArchive::Ptr original_archive{ ZipArchive::Create (original_archive_stream) };
		if (nullptr == original_archive) {
			MessageBox (NULL, TEXT ("Что-то пошло не так"), TEXT ("Ошибка"),
				MB_OK | MB_ICONERROR);
			return EXIT_FAILURE;
		}

		original_archive_file.close ();

		ZipArchive::Ptr translate_archive{ ZipArchive::Create (_archive) };
		if (nullptr == translate_archive) {
			MessageBox (NULL, TEXT ("Что-то пошло не так"), TEXT ("Ошибка"),
				MB_OK | MB_ICONERROR);
			return EXIT_FAILURE;
		}

		const size_t entries = translate_archive->GetEntriesCount ();
		const size_t entries_len{ num_len (entries) };

		console::color.set (console::t_color::DARKGRAY);

		RECT rect;
		GetClientRect (m_wnd.hwnd (), &rect);

		const int x{ 0 };
		const int y{ rect.bottom - 30 };
		const int width{ rect.right };
		const int height{ 30 };

		ea::window progress_bar{
			CreateWindowEx (
				0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
				x, y, width, height, m_wnd.hwnd (), NULL, m_wnd.hinstance (), NULL
			) };
		
		PostMessage (progress_bar.hwnd (), PBM_SETRANGE, 0, MAKELPARAM (0, entries));
		PostMessage (progress_bar.hwnd (), PBM_SETSTEP, 1, 0);

		for (size_t q{ 0 }; q != entries; ++q) {
			auto txt_entry = translate_archive->GetEntry (q);

			if (std::filesystem::path{ txt_entry->GetName () }.extension ().compare (".txt")) {
				PostMessage (progress_bar.hwnd (), PBM_STEPIT, 0, 0);
				continue;
			}

			std::cout << "> ";

			console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
			std::cout << ' '
				<< std::setfill ('0')
				<< std::setw (entries_len)
				<< std::right
				<< q + 1
				<< ' ';

			console::color = (int) console::t_color::WHITE | (int) console::b_color::BLACK;
			std::cout << ' '
				<< std::setfill (' ')
				<< std::setw (70)
				<< std::left
				<< txt_entry->GetName ()
				<< ' ';

			console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
			std::cout << ' ' << txt_entry->GetSize () << ' ';

			console::color.set (console::t_color::DARKGRAY);
			std::cout << " bytes\n";

			const auto fullname{ "../bin/Table/" + txt_entry->GetName () };

			std::filesystem::path res_entry_path{ fullname };
			std::string name{ res_entry_path.replace_extension ("res").string () };
			auto res_entry{ original_archive->GetEntry (name) };
			if (nullptr == res_entry) { 
				std::cout << "> " << name << " skipped\n";
				continue; 
			}
			
			res_entry->SetPassword (DATA12PASSWD);
			std::stringstream res{ std::ios::in | std::ios::out | std::ios::binary };
			utils::stream::copy (*res_entry->GetDecompressionStream (), res);

			std::stringstream txt{ std::ios::in | std::ios::out };
			utils::stream::copy (*txt_entry->GetDecompressionStream (), txt);

			auto new_data{ processor::crypto <tools::coder::encode> (txt_entry->GetName (), res, txt) };

			original_archive->RemoveEntry (name);

			res_entry = original_archive->CreateEntry (name);
			res_entry->SetPassword (DATA12PASSWD);
			res_entry->UseDataDescriptor ();
			res_entry->SetCompressionStream (new_data,
				DeflateMethod::Create (),
				ZipArchiveEntry::CompressionMode::Immediate);

			PostMessage (progress_bar.hwnd (), PBM_STEPIT, 0, 0);
		}

		DBG_MESSAGE ("[game archive] open");
		const auto new_arch_path{ path / "data12.v"};
		std::ofstream new_archive_file{ new_arch_path, std::ios::out | std::ios::binary };

		if (!new_archive_file.is_open ()) {
			MessageBox (NULL, TEXT ("Что-то пошло не так"), TEXT ("Ошибка"),
				MB_OK | MB_ICONERROR);
			return EXIT_FAILURE;
		}
		DBG_MESSAGE_OK ();
		
		std::stringstream raw_file_stream{ std::ios::out | std::ios::in | std::ios::binary };
		
		original_archive->WriteToStream (raw_file_stream);
		std::string raw_file{ raw_file_stream.str () };

		DBG_MESSAGE ("encoding...");
		std::for_each (std::execution::par_unseq, std::begin (raw_file), std::end (raw_file), [](auto& e) noexcept {
			e ^= 0x55;
		});
		DBG_MESSAGE_OK ();
		
		new_archive_file.write (std::data (raw_file), std::size (raw_file));
		new_archive_file.close ();

		MessageBox (NULL, TEXT ("Готово! Вы восхитительны!"), TEXT ("SoulWorker"), MB_OK | MB_ICONINFORMATION);
		return EXIT_SUCCESS;
	}

	void loop (void) noexcept {
		m_wnd.loop ([this](void) noexcept {

		});
	}

	int work (void) noexcept {
		auto thread{ this->load_file () };
		std::filesystem::path path{ this->select_folder () };

		if (path.empty ()) { return EXIT_SUCCESS; }

		return this->patch (
			std::forward <std::filesystem::path> (path),
			std::forward <std::stringstream> (thread.get ()));
	}

	app (void) noexcept : m_wnd{
		400, 200, TEXT ("sw_patcher_steam.exe"), 
		[](const HWND _wnd, const UINT _msg, const WPARAM _wParams, const LPARAM _lParam) noexcept -> LRESULT {
			switch (_msg) {
			case WM_CLOSE:
				DestroyWindow (_wnd);
			return 0;
			case WM_DESTROY:
				PostQuitMessage (EXIT_SUCCESS);
			return 0;
			}
			return DefWindowProc (_wnd, _msg, _wParams, _lParam);
		}
	},
	m_worker{ std::async (std::launch::async, [this](void) noexcept {
		PostMessage (m_wnd.hwnd (), WM_CLOSE, this->work (), 0);
	}) }{}

	~app (void) noexcept {}
private:
	ea::window m_wnd;
	std::future <void> m_worker;
};

int main (int argc, char * argv) noexcept
{
	app swp_pather_steam;
	swp_pather_steam.loop ();

	return EXIT_SUCCESS;
}