// ADG Orange

#include "headers.hpp"
#include "../swp_steam_lib/header.hpp"

class FUNCS {
public:

	INT static unpack(_In_ INT argc, _In_ TCHAR* argv[]) noexcept {
		if (argc < 2) { return EXIT_FAILURE; }

		const std::filesystem::path path{ argv[0] };

		tools::fs::file <std::ifstream> ([&](auto& _ifs) noexcept {
			auto stream{ tools::reader::res_archive (_ifs) };
			ZipArchive::Ptr archive{ ZipArchive::Create (stream) };
			if (nullptr == archive) { return EXIT_FAILURE; }

			std::filesystem::create_directories (argv[1]);

			size_t entries_len{ 0 };
			auto const write = [&](const size_t& q, const ZipArchiveEntry::Ptr& e) noexcept {
				std::filesystem::path p{ std::filesystem::path (argv[1]) / e->GetName () };

				console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
				std::cout << ' ' << std::setfill ('0') << std::setw (entries_len) << q << ' ';

				console::color = (int) console::t_color::WHITE | (int) console::b_color::BLACK;
				std::cout << ' ' << p << ' ';

				std::ofstream f{ p, std::ios::binary | std::ios::trunc };
				if (f.fail ()) {
					DBG_MESSAGE_ER ();
					return;
				}
				console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
				std::cout << " ok! ";

				console::color.set (console::t_color::DARKGRAY);
				std::cout << "\n";

				e->SetPassword (DATA12PASSWD);
				utils::stream::copy (*e->GetDecompressionStream (), f);

				f.flush ();
				f.close ();
			};

			if (argc > 2) {
				entries_len = num_len ((size_t) argc - 2);
				for (size_t q{ 2 }, end{ (size_t) argc }; q != end; ++q) {
					TCHAR * f = argv[q];
					auto e{ archive->GetEntry (f) };

					if (nullptr == e) {
						console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
						std::cout << ' ' << std::setfill ('0') << std::setw (entries_len) << q - 1 << ' ';

						console::color = (int) console::t_color::WHITE | (int) console::b_color::BLACK;
						std::cout << ' ' << argv[q] << ' ';

						DBG_MESSAGE_ER ();
						continue;
					}

					write (q - 1, e);
				}

				return EXIT_SUCCESS;
			}

			entries_len = num_len (archive->GetEntriesCount ());
			for (size_t q{ 0 }, end{ archive->GetEntriesCount () }; q != end; ++q) {
				write (q + 1, archive->GetEntry (q));
			}

			return EXIT_SUCCESS;
		}, path, std::ifstream::binary);
		
		return EXIT_FAILURE;
	}

	INT static pack(_In_ INT argc, _In_ TCHAR* argv[]) noexcept {
		return EXIT_SUCCESS;
	}

	INT static decode(_In_ INT argc, _In_ TCHAR* argv[]) noexcept {
		if (argc < 2) { return EXIT_FAILURE; }

		std::filesystem::path res_file{ argv[0] };
		std::filesystem::path txt_file{ argv[1] };

		tools::fs::file <std::ofstream> ([&](auto& _ofs) noexcept -> void {
			tools::fs::file <std::ifstream> ([&](auto& _ifs) noexcept -> void {
				DBG_MESSAGE ("reading...");
				std::istringstream res{ tools::reader::file (_ifs) };
				DBG_MESSAGE_OK ();

				std::stringstream txt;
				auto result{ processor::crypto <tools::coder::decode> (res_file, res, txt) };
				_ofs << result.str () << std::flush;
			}, res_file, std::ios::binary);
		}, txt_file, std::ios::trunc);

		return EXIT_SUCCESS;
	}

	INT static encode (_In_ INT argc, _In_ TCHAR* argv[]) noexcept {
		if (argc < 2) { return EXIT_FAILURE; }

		std::filesystem::path res_file{ argv[0] };
		std::filesystem::path txt_file{ argv[1] };

		tools::fs::file <std::fstream> ([&](auto& _res_fs) noexcept -> void {
			tools::fs::file <std::ifstream> ([&](auto& _txt_ifs) noexcept -> void {
				DBG_MESSAGE ("reading soulworker text file...");
				std::stringstream res{ tools::reader::file (_res_fs) };
				DBG_MESSAGE_OK ();

				DBG_MESSAGE ("reading translate text file...");
				std::istringstream txt{ tools::reader::file (_txt_ifs) };
				DBG_MESSAGE_OK ();

				auto result{ processor::crypto <tools::coder::encode> (res_file, res, txt) };

				DBG_MESSAGE ("writing to soulworker text file...");
				/*std::filesystem::resize_file (res_file, 0);
				_res_fs.seekp (0);

				_res_fs << result.rdbuf ();*/

				DBG_MESSAGE_OK ();
			}, txt_file);
		}, res_file, std::ios::binary | std::ios::in | std::ios::out);
		return EXIT_SUCCESS;
	}
	
	INT static list(_In_ INT argc, _In_ TCHAR* argv[]) noexcept {
		if (!argc) { return EXIT_FAILURE; }
		 
		const std::filesystem::path path{ argv[0] };

		tools::fs::file <std::ifstream> ([&](auto& _ifs) noexcept {
			auto stream{ tools::reader::res_archive (_ifs) };
			if (!stream.rdbuf ()->in_avail ()) { return EXIT_FAILURE; }

			auto archive{ ZipArchive::Create (stream) };
			if (nullptr == archive) { return EXIT_FAILURE; }

			size_t entries = archive->GetEntriesCount ();

			console::color.set (console::t_color::DARKGRAY);
			std::cout << "> ";

			console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
			std::cout << ' ' << path.filename () << ' ';

			console::color = (int) console::b_color::BLACK | (int) console::t_color::WHITE;
			std::cout << " is open and countains ";

			console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
			std::cout << ' ' << entries << ' ';

			console::color = (int) console::b_color::BLACK | (int) console::t_color::WHITE;
			std::cout << " entries\n";

			const size_t entries_len{ num_len (entries) };

			console::color.set (console::t_color::DARKGRAY);
			for (size_t q{ 0 }; q != entries; q++) {
				auto e = archive->GetEntry (q);

				std::cout << "> ";

				console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
				std::cout << ' ' << std::setfill ('0') << std::setw (entries_len) << std::right << q + 1 << ' ';

				console::color = (int) console::t_color::WHITE | (int) console::b_color::BLACK;
				std::cout << ' ' << std::setfill (' ') << std::setw (70) << std::left << e->GetFullName () << ' ';

				console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
				std::cout << ' ' << e->GetSize () << ' ';

				console::color.set (console::t_color::DARKGRAY);
				std::cout << " bytes\n";
			}

			return EXIT_SUCCESS;
		}, path, std::ifstream::binary);

		return EXIT_FAILURE;		
	}

	INT static angel_patch (_In_ INT argc, _In_ TCHAR* argv[]) noexcept 
	{
		const auto WEB_URL = TEXT ("https://github.com/AngelDevelopersGroup/soulworker-steam-ru-translate/archive/master.zip");
		
		DBG_MESSAGE ("make conenction");
		const HINTERNET WEB_CONNECT = 
			InternetOpen (TEXT ("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:54.0) Gecko/20100101 Firefox/54.0"), 
				INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (!WEB_CONNECT) {
			DBG_MESSAGE_ER ();
			return 0;
		}
		DBG_MESSAGE_OK ();

		std::stringstream stringstream{ std::ios::in | std::ios::out | std::ios::binary };

		DBG_MESSAGE ("open site");
		HINTERNET WEB_ADDRESS = InternetOpenUrl (WEB_CONNECT, WEB_URL, NULL, 0, INTERNET_FLAG_KEEP_CONNECTION, 0);
		if (!WEB_ADDRESS) {
			DBG_MESSAGE_ER ();
			InternetCloseHandle (WEB_CONNECT);
			return 0;
		}
		DBG_MESSAGE_OK ();

		constexpr DWORD _SIZE{ 307200 };
		DWORD DATA_SIZE{ _SIZE };
		std::unique_ptr <char[]> _DATA_RECIEVED{ std::make_unique <char[]> (DATA_SIZE) };

		DBG_MESSAGE ("download last translation files");
		do {
			DWORD NO_BYTES_READ{ 0 };
			if (InternetReadFile (WEB_ADDRESS, _DATA_RECIEVED.get(), DATA_SIZE, &NO_BYTES_READ)) {
				if (NO_BYTES_READ == 0) { break; }

				stringstream << std::string (_DATA_RECIEVED.get (), NO_BYTES_READ);
			} else {
				if (GetLastError () != ERROR_INSUFFICIENT_BUFFER) {
					DBG_MESSAGE_ER ();
					break;
				}

				DATA_SIZE += _SIZE;
				_DATA_RECIEVED = std::make_unique <char[]> (DATA_SIZE);
			}
		} while (true);
		DBG_MESSAGE_OK ();

		InternetCloseHandle (WEB_ADDRESS);
		InternetCloseHandle (WEB_CONNECT);
		
		DBG_MESSAGE ("open archive");
		auto archive{ ZipArchive::Create (stringstream) };
		if (nullptr == archive) {
			DBG_MESSAGE_ER (); 
			return EXIT_FAILURE; 
		}
		DBG_MESSAGE_OK ();

		const size_t entries = archive->GetEntriesCount ();

		const size_t entries_len{ num_len (entries) };
		console::color.set (console::t_color::DARKGRAY);
		for (size_t q{ 0 }; q != entries; q++) {
			auto e = archive->GetEntry (q);

			if (std::filesystem::path{ e->GetName () }.extension ().compare (".txt")) { continue; }

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
				<< e->GetName ()
				<< ' ';

			console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
			std::cout << ' ' << e->GetSize () << ' ';

			console::color.set (console::t_color::DARKGRAY);
			std::cout << " bytes\n";
		}

		std::ofstream file{ "file.zip", std::ios::binary };
		file << stringstream.str ();
		file.close ();

		return EXIT_SUCCESS;
	}
};

namespace an {
#ifdef _UNICODE
	using tstring = std::wstring;
#else
	using tstring = std::string;
#endif
}

std::unordered_map <an::tstring, std::function <INT(_In_ INT, _In_ TCHAR*[])>> funcs;
int work(_In_ INT argc, _In_ TCHAR* argv[]) noexcept {

	// unpack
	funcs.emplace(TEXT ("-u"), FUNCS::unpack);

	// pack
	funcs.emplace(TEXT ("-p"), FUNCS::pack);

	// decode
	funcs.emplace(TEXT ("-d"), FUNCS::decode);

	// encode
	funcs.emplace(TEXT ("-e"), FUNCS::encode);

	// list
	funcs.emplace (TEXT ("-l"), FUNCS::list);

	// angel patch
	funcs.emplace (TEXT ("-ap"), FUNCS::angel_patch);
	
	const auto func{ funcs.find(argv[1]) };
	if (std::cend(funcs) == func) { 
		std::cout << "Error";
		return EXIT_FAILURE;
	}

	return func->second(argc - 2, (argv + 2));
}


static std::string reader_file (std::ifstream& ifs) noexcept {
	return std::string{ std::istreambuf_iterator <char> (ifs), {} };
}

int main(_In_ INT argc, _In_ TCHAR* argv[]) noexcept {
	if (argc < 2) {
		auto const caption = [](const char * str) noexcept {
			console::color = (int)console::t_color::DARKGRAY | (int)console::b_color::BLACK;
			std::cout << "> ";

			console::color = (int)console::b_color::WHITE | (int)console::t_color::BLACK;
			std::cout << " " << str << " ";

			console::color = (int)console::t_color::DARKGRAY | (int)console::b_color::BLACK;
			std::cout << '\n';
		};

		auto const line = [](const char * name, const char * info) noexcept {
			console::color = (int)console::t_color::DARKGRAY | (int)console::b_color::BLACK;
			std::cout << "> ";

			console::color = (int)console::b_color::WHITE | (int)console::t_color::BLACK;
			std::cout << " " << name << " ";

			console::color = (int)console::t_color::DARKGRAY | (int)console::b_color::BLACK;
			std::cout << "\n> swp_steam " << info << '\n';
		};

		caption("usage:");
		line("unpack", "-u <archive> <path-to-unpack> [<file> <path>...]\n");

		line("pack", "-p <archive> <path-to-files> or <files>\n");

		line("decode", "-d <path-to-files> or <files>\n");

		line ("encode", "-e <path-to-files> or <files>\n");

		line ("list", "-l <path-to-archive>\n\n");

		caption("examples:");
		line("unpack", "-u datas/data12.v datas/data12/\n> swp_steam -u datas/data24.v World/Navmesh/T01_TUTORIAL.hkt\n");

		line("pack", "-p datas/data12.v datas/data12/\n> swp_steam -p datas/data12.v tb_UI_Img ../bin/Table/tb_UI_Img tb_Help ../bin/Table/tb_Help\n");

		line("decode", "-d datas/data12/\n> swp_steam -d datas/data12/tb_Achievement_begin datas/data12/tb_Akashic_Parts datas/data12/tb_Buff\n");

		line("encode", "-e datas/data12/\n> swp_steam -e datas/data12/tb_Achievement_begin datas/data12/tb_Akashic_Parts datas/data12/tb_Buff\n");
		
		line ("list", "-l datas/data12.v");
		return EXIT_SUCCESS;
	}

	int ret = work(argc, argv);

	console::color.set(console::t_color::DARKGRAY);
	//system("pause");
	return ret;
	

	return EXIT_SUCCESS;
}
