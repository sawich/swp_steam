#pragma once

#define DATA12PASSWD "@PDB@?LoadBinaryVersion"
#include "headers.hpp"


#ifdef _CONSOLE
#define DBG_MESSAGE(msg) console::color.set(console::t_color::DARKGRAY);std::cout << "> " << msg << "   "
#define DBG_MESSAGE_OK() console::color.set(console::t_color::GREEN);std::cout << "ok!\n"
#define DBG_MESSAGE_ER() console::color.set(console::t_color::RED);std::cout << "error!\n"
#else
#define DBG_MESSAGE(msg)
#define DBG_MESSAGE_OK()
#define DBG_MESSAGE_ER()
#endif

static size_t num_len (size_t _num) noexcept {
	static size_t sizeTable[] = { 9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999, (size_t) std::numeric_limits<int>::max () };
	for (size_t i = 0; ; i++) {
		if (_num <= sizeTable[i]) {
			return i + 1;
		}
	}
	return 0;
}

class tools {
	
public:
	void static replace (
		std::string& subject,
		const std::string& search,
		const std::string& replace
	) noexcept {
		size_t pos = 0;
		while ((pos = subject.find (search, pos)) != std::string::npos) {
			subject.replace (pos, search.length (), replace);
			pos += replace.length ();
		}
	}
	class fs {
	public:
		template <typename T, typename ...ARGS>
		class stream_t : public T {
		public:
			stream_t (ARGS&&... _args) noexcept : T (std::forward <ARGS> (_args)...) {}
			~stream_t (void) noexcept {
				if (T::is_open ()) {
					T::close ();
				}
			}
		};

		template <typename T, typename F, typename PATH, typename ...ARGS>
		static void file (F&& _func, PATH&& _path, ARGS &&... _args) noexcept {
			console::color = (int) console::t_color::WHITE | (int) console::b_color::BLACK;
			std::cout << "> " << _path << ' ';

			stream_t <T, PATH, ARGS...> stream{ std::forward <PATH> (_path), std::forward <ARGS> (_args)... };
			if (stream.fail ()) {
				DBG_MESSAGE_ER ();
				return;
			}

			console::color = (int) console::b_color::WHITE | (int) console::t_color::BLACK;
			std::cout << " was opened ";

			console::color = (int) console::t_color::WHITE | (int) console::b_color::BLACK;
			std::cout << '\n';

			_func (stream);
		}
	};

	class hash {
	public:
		static constexpr size_t djb (std::string_view toHash) noexcept {
			size_t hash = static_cast <size_t> (5381);

			for (char c : toHash) {
				hash = ((hash << static_cast <size_t> (5)) + hash) + c;
			}

			return hash;
		}
	};

	class reader {
	public:
		template <typename F>
		static std::string file (F&& _ifs) noexcept {
			_ifs.seekg (0, std::ios::end);
			const size_t size{ static_cast <size_t> (_ifs.tellg ()) };
			_ifs.seekg (0, std::ios::beg);

			std::string data;
			data.resize (size);

			_ifs.read (std::data (data), size);

			return data;
		}

		template <typename F>
		static std::istringstream res_archive (F&& _ifs) noexcept {
			std::istringstream iss{};

			DBG_MESSAGE ("reading...");
			std::string raw_file (reader::file (_ifs));
			DBG_MESSAGE_OK ();

			DBG_MESSAGE ("decoding...");
			std::for_each (std::execution::par_unseq, std::begin (raw_file), std::end (raw_file), [](auto& e) noexcept {
				e ^= 0x55;
			});
			DBG_MESSAGE_OK ();
			iss = std::istringstream{ raw_file };
			return iss;
		}
	};

	using converter = std::wstring_convert <std::codecvt_utf8_utf16 <wchar_t>, wchar_t>;

	template <typename T>
	static auto sort (T && _unordered_map) noexcept {
		return std::map <T::key_type, T::mapped_type> { std::begin (_unordered_map), std::end (_unordered_map) };
	}

	class data {
	public:
		class from_raw {
		public:
			class reader {
			public:
				template <typename T, typename R>
				static T byte (R && _ifs) noexcept {
					T readed;
					_ifs.read ((char *) &readed, sizeof (T));
					return readed;
				}

				template <typename T, typename R>
				static T integer (R && _ifs) noexcept {
					return byte <T> (_ifs);
				}

				template <typename T, typename R>
				static T string (R && _ifs) noexcept {
					uint16_t size;
					_ifs.read ((char *) &size, sizeof (uint16_t));
					T readed;
					readed.resize (size);
					_ifs.read ((char *) std::data (readed), size * sizeof (T::value_type));
					return readed;
				}
			};
		};
		class from_txt {
		public:
			class reader {
			public:
				template <typename T, typename R>
				static T byte (R && _ifs) noexcept {
					std::string txt;
					std::getline (_ifs, txt);
					return txt[0];
				}

				template <typename T, typename R>
				static T integer (R && _ifs) noexcept {
					std::string txt;
					std::getline (_ifs, txt);
					T data{ static_cast <T> (std::stoi (txt)) };
					return data;
				}

				template <typename T, typename R>
				static std::wstring string (R && _ifs) noexcept {
					std::string txt;
					std::getline (_ifs, txt);

					tools::replace (txt, "\\n", "\n");

					tools::converter convertor;
					std::wstring data{ convertor.from_bytes (txt) };

					return data;
				}
			};

			class writer {
			public:
				template <typename T, typename O, typename GT>
				static void byte (O && _ofs, GT _data) noexcept {
					_ofs.write ((char *) &_data, sizeof (GT));
				}

				template <typename T, typename O, typename GT>
				static void integer (O && _ofs, GT _data) noexcept {
					T data{ static_cast <T> (_data) };
					_ofs.write ((char *) &data, sizeof (T));
				}

				template <typename T, typename O>
				static void string (O && _ofs, T && _data) noexcept {
					uint16_t size{ static_cast <uint16_t> (std::size (_data)) };
					_ofs.write ((char *) &size, sizeof (size));
					_ofs.write ((char *) std::data (_data), size * sizeof (std::remove_reference <T>::type::value_type));
				}
			};
		};
		class escape {
		public:
			template <typename T>
			static std::string string (T && _data) noexcept {
				tools::converter converter;
				std::string data{ converter.to_bytes (_data) };
				tools::replace (data, "\n", "\\n");
				return data;
			}
		};
	};

	class coder {
	public:
		class encode {};
		class decode {};
	};
};


struct ubyte_array {};
struct ubyte_array_ignore {};
struct ubyte {};

template <typename ...Ts>
struct typelist {};

class processor_decoder {
	template <
		typename RES_STREAM,
		typename RETURN_STREAM,
		typename T
	>
		static void decoder_run (
			RES_STREAM& _res_file,
			RETURN_STREAM& _ostringstream
		) noexcept {
		if constexpr (std::is_same_v <T, ubyte_array>) {
			auto raw_str{ tools::data::from_raw::reader::string <std::wstring> (_res_file) };
			auto str{ tools::data::escape::string (raw_str) };
			_ostringstream << str << '\n';
		} else if constexpr (std::is_same_v <T, ubyte_array_ignore>) {
			auto len{ tools::data::from_raw::reader::integer <uint16_t> (_res_file) };
			_res_file.seekg (sizeof (wchar_t) * len, std::ios::cur);
		} else {
			_res_file.seekg (sizeof (T), std::ios::cur);
		}
	}

	template <
		typename RES_STREAM,
		typename RETURN_STREAM,
		template <typename...> class List,
		typename ...Ts
	>
		static void decoder_impl (
			RES_STREAM& _res_file,
			RETURN_STREAM& _ostringstream,
			List <Ts...> &&
		) noexcept {
		(decoder_run <RES_STREAM, RETURN_STREAM, Ts> (_res_file, _ostringstream), ...);
	}
public:
	template <
		typename RES_STREAM,
		typename RETURN_STREAM,
		typename Id,
		typename List
	>
		static void decode (
			RES_STREAM& _res_file,
			RETURN_STREAM& _ostringstream
		) noexcept {
		_ostringstream << tools::data::from_raw::reader::integer <Id> (_res_file) << '\n';
		decoder_impl (_res_file, _ostringstream, List{});
	}
};

class processor_encoder_reader {
	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename T>
		static void encoder_reader_run (
			InStream& _txt_file,
			OutStream& _memory_line
		) noexcept {
		if constexpr (std::is_same_v <T, ubyte_array>) {
			std::wstring str{ tools::data::from_txt::reader::string <std::wstring> (_txt_file) };
			tools::data::from_txt::writer::string (_memory_line, str);
		} else {
			static_assert("unknown type");
		}
	}

	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		template <typename...> class List,
		typename ...Ts >
		static void encoder_reader_impl (
			InStream& _txt_file,
			OutStream& _memory_line,
			List <Ts...> &&
		) noexcept {
		(encoder_reader_run <InStream, OutStream, EncoderReader, Ts> (_txt_file, _memory_line), ...);
	}
public:
	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename List>
		static void encoder_reader (
			InStream& _txt_file,
			OutStream& _memory_line
		) noexcept {
		encoder_reader_impl <InStream, OutStream, EncoderReader> (_txt_file, _memory_line, List{});
	}
};

class processor_encoder {
	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename T>
		static uint64_t encoder_run (
			InStream& _txt_file,
			OutStream& _memory_line
		) noexcept {
		if constexpr (std::is_same_v <T, ubyte_array> || std::is_same_v <T, ubyte_array_ignore>) {
			std::wstring str{ EncoderReader::template string <std::wstring, InStream> (_txt_file) };
			tools::data::from_txt::writer::string (_memory_line, str);

			uint64_t hash_sum{ std::size (str) };

			BYTE * p = reinterpret_cast <BYTE *> (std::data (str));
			for (uint16_t q{ 0 }, e{ static_cast <uint16_t> (hash_sum * 2) }; q < e; ++q) {
				hash_sum += *(p + q);
			}

			return hash_sum;
		} else if constexpr (std::is_same_v <T, uint32_t>) {
			uint32_t number{ EncoderReader::template integer <uint32_t> (_txt_file) };
			tools::data::from_txt::writer::integer <uint32_t> (_memory_line, number);
			return number;
		} else if constexpr (std::is_same_v <T, uint16_t>) {
			uint16_t number{ EncoderReader::template integer <uint16_t> (_txt_file) };
			tools::data::from_txt::writer::integer <uint16_t> (_memory_line, number);
			return number;
		} else if constexpr (std::is_same_v <T, ubyte>) {
			BYTE number{ EncoderReader::template byte <BYTE> (_txt_file) };
			tools::data::from_txt::writer::byte <BYTE> (_memory_line, number);
			return number;
		} else {
			static_assert("unknown type");
		}
	}

	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		template <typename...> class List,
		typename ...Ts >
		static uint64_t encoder_impl (
			InStream& _txt_file,
			OutStream& _memory_line,
			List <Ts...> &&
		) noexcept {
		return (encoder_run <InStream, OutStream, EncoderReader, Ts> (_txt_file, _memory_line) + ...);
	}
public:
	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename List>
		static uint64_t encoder (
			InStream& _txt_file,
			OutStream& _memory_line
		) noexcept {
		return encoder_impl <InStream, OutStream, EncoderReader> (_txt_file, _memory_line, List{});
	}
};

class processor_replacer {
	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename NewData,
		typename T
	>
		static uint64_t encoder_replacer_run (
			InStream& _res_file,
			OutStream& _memory_line,
			NewData& _new_data
		) noexcept {
		if constexpr (std::is_same_v <T, ubyte_array>) {
			std::wstring str{ tools::data::from_raw::reader::string <std::wstring> (_new_data) };
			tools::data::from_raw::reader::string <std::wstring> (_res_file);
			tools::data::from_txt::writer::string (_memory_line, str);

			uint64_t hash_sum{ std::size (str) };

			BYTE * p = reinterpret_cast <BYTE *> (std::data (str));
			for (uint16_t q{ 0 }, e{ static_cast <uint16_t> (hash_sum * 2) }; q < e; ++q) {
				hash_sum += *(p + q);
			}

			return hash_sum;
		} else if constexpr (std::is_same_v <T, ubyte_array_ignore>) {
			std::wstring str{ tools::data::from_raw::reader::string <std::wstring> (_res_file) };			
			tools::data::from_txt::writer::string (_memory_line, str);

			uint64_t hash_sum{ std::size (str) };

			BYTE * p = reinterpret_cast <BYTE *> (std::data (str));
			for (uint16_t q{ 0 }, e{ static_cast <uint16_t> (hash_sum * 2) }; q < e; ++q) {
				hash_sum += *(p + q);
			}

			return hash_sum;
		} else if constexpr (std::is_same_v <T, uint32_t>) {
			uint32_t number{ EncoderReader::template integer <uint32_t> (_res_file) };
			tools::data::from_txt::writer::integer <uint32_t> (_memory_line, number);
			return number;
		} else if constexpr (std::is_same_v <T, uint16_t>) {
			uint16_t number{ EncoderReader::template integer <uint16_t> (_res_file) };
			tools::data::from_txt::writer::integer <uint16_t> (_memory_line, number);
			return number;
		} else if constexpr (std::is_same_v <T, ubyte>) {
			BYTE number{ EncoderReader::template byte <BYTE> (_res_file) };
			tools::data::from_txt::writer::byte <BYTE> (_memory_line, number);
			return number;
		}
	}

	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename NewData,
		template <typename...> class List,
		typename ...Ts
	>
		static uint64_t encoder_replacer_impl (
			InStream& _res_file,
			OutStream& _memory_line,
			NewData& _new_data,
			List <Ts...> &&
		) noexcept {
		return (encoder_replacer_run <InStream, OutStream, EncoderReader, NewData, Ts> (_res_file, _memory_line, _new_data) + ...);
	}
public:
	template <
		typename InStream,
		typename OutStream,
		class EncoderReader,
		typename List,
		typename NewData>
		static uint64_t encoder_replacer (
			InStream& _res_file,
			OutStream& _memory_line,
			NewData& _new_data
		) noexcept {
		return encoder_replacer_impl <InStream, OutStream, EncoderReader, NewData> (_res_file, _memory_line, _new_data, List{});
	}
};

class processor {
public:
	template <
		typename RES_STREAM,
		typename Id,
		typename ...Ts
	>
		static std::stringstream decoder (
			RES_STREAM&& _res_file
		) noexcept {
		std::stringstream stringstream{ std::ios::out };

		const auto count{ tools::data::from_raw::reader::integer <uint32_t> (_res_file) };
		stringstream << count << '\n';

		

		for (uint32_t q{ 0 }; q < count; ++q) {
			processor_decoder::decode <RES_STREAM, std::stringstream, Id, typelist <Ts...>> (_res_file, stringstream);
		}

		auto hash{ tools::data::from_raw::reader::string <std::string> (_res_file) };

		console::color.set (console::t_color::DARKGRAY);
		std::cout << "\n> file hash ";

		console::color.set (console::t_color::GREEN);
		std::cout << hash << '\n';

		return stringstream;
	}


	template <
		typename RES_STREAM,
		typename TXT_STREAM,
		typename Id,
		typename ...Ts
	>
		static std::stringstream encoder (
			RES_STREAM&& _res_file,
			TXT_STREAM&& _txt_file
		) noexcept {
		std::stringstream stringstream{ std::ios::binary | std::ios::in | std::ios::out };
		uint64_t hash_sum{ 0 };
		std::unordered_map <Id, std::stringstream> txt_data;
		
		{
			auto count{ tools::data::from_txt::reader::integer <uint32_t> (_txt_file) };
			txt_data.reserve (count);

			// read txt file
			for (uint32_t q{ 0 }; q < count; ++q) {
				std::stringstream memory_line{ std::ios::binary | std::ios::in | std::ios::out };

				auto id{ tools::data::from_txt::reader::integer <Id> (_txt_file) };

				processor_encoder_reader::encoder_reader <
					TXT_STREAM,
					std::stringstream,
					tools::data::from_txt::reader,
					typelist <Ts...>
				> (_txt_file, memory_line);

				txt_data.emplace (id, std::move (memory_line));
			}
		}
		
		auto count{ tools::data::from_raw::reader::integer <uint32_t> (_res_file) };
		tools::data::from_txt::writer::integer <uint32_t> (stringstream, count);

		// replace and write
		for (uint32_t q{ 0 }; q < count; ++q) {
			auto id{ tools::data::from_raw::reader::integer <Id> (_res_file) };

			tools::data::from_txt::writer::integer <Id> (stringstream, id);
			hash_sum += id;

			if (const auto find{ txt_data.find (id) }; std::cend (txt_data) != find) {
				hash_sum += processor_replacer::encoder_replacer <
					RES_STREAM,
					std::stringstream,
					tools::data::from_raw::reader,
					typelist <Ts...>,
					std::stringstream
				> (_res_file, stringstream, find->second);
				continue;
			}

			hash_sum += processor_encoder::encoder <
				RES_STREAM,
				std::stringstream,
				tools::data::from_raw::reader,
				typelist <Ts...>
			> (_res_file, stringstream);
		}

		DBG_MESSAGE_OK ();

		console::color.set (console::t_color::DARKGRAY);
		std::cout << "> old hash ";

		console::color.set (console::t_color::GREEN);

		auto hash{ tools::data::from_raw::reader::string <std::string> (_res_file) };
		std::cout << hash << '\n';


		console::color.set (console::t_color::DARKGRAY);

		auto calulated_hash{ md5 (std::to_string (hash_sum)) };
		std::cout << "> new hash ";

		console::color.set (console::t_color::GREEN);
		std::cout << calulated_hash << '\n';

		tools::data::from_txt::writer::string (stringstream, calulated_hash);

		return stringstream;
	}

	template <
		typename CODER,
		typename RES_STREAM,
		typename TXT_STREAM,
		typename ...TYPES
	>
		static std::stringstream work (
			RES_STREAM&& res_file,
			TXT_STREAM&& txt_file
		) noexcept {
		if constexpr (std::is_same_v <CODER, tools::coder::decode>) {
			DBG_MESSAGE ("converting...");
			return ::processor::decoder <RES_STREAM, TYPES...> (res_file);
		}
		DBG_MESSAGE ("converting...");
		return ::processor::encoder <RES_STREAM, TXT_STREAM, TYPES...> (res_file, txt_file);
	}

	template <
		typename CODER,
		typename RES_STREAM,
		typename TXT_STREAM
	>
		static std::stringstream crypto (
			const std::filesystem::path& file_name,
			RES_STREAM&& res_file,
			TXT_STREAM&& txt_file
		) noexcept {
#define LANG_GEN(name) \
		case tools::hash::djb(##name""):\
		case tools::hash::djb(##name"_eng"):\
		case tools::hash::djb(##name"_ESP"):\
		case tools::hash::djb(##name"_FRE"):\
		case tools::hash::djb(##name"_GER"):\
		case tools::hash::djb(##name"_ITL"):\
		case tools::hash::djb(##name"_kor"):\
		case tools::hash::djb(##name"_pol")

		switch (tools::hash::djb (file_name.stem ().string ())) {
			LANG_GEN ("tb_Achievement_Script") : {
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array, ubyte_array> (res_file, txt_file);
			}
			LANG_GEN ("tb_Booster_Script") :
				break;
			LANG_GEN ("tb_Buff_Script") :
				break;
			LANG_GEN ("tb_Cinema_String") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint16_t, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_Cutscene_String") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_item_script") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte_array, ubyte, ubyte, ubyte, ubyte, ubyte, ubyte_array, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_LeagueSkill_Script") :
				break;
			LANG_GEN ("tb_Help") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, uint16_t, uint16_t, uint32_t, uint32_t, ubyte_array, uint32_t> (res_file, txt_file);
			LANG_GEN ("tb_Monster_script") :
				break;
			LANG_GEN ("tb_NPC_Script") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_Quest_Script") :
				break;
			LANG_GEN ("tb_Shop_String") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_Skill_Script") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint16_t, ubyte_array, ubyte_array, ubyte_array, ubyte_array_ignore, uint32_t, uint32_t, ubyte_array_ignore, ubyte_array_ignore> (res_file, txt_file);
			LANG_GEN ("tb_soul_metry_string") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_Speech_String") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_SystemMail") :
				break;
			LANG_GEN ("tb_Title_String") :
				break;
			LANG_GEN ("tb_Tooltip_String") :
				break;
			LANG_GEN ("tb_UI_Img") :
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte_array> (res_file, txt_file);
			LANG_GEN ("tb_UI_String") : {
				return work <CODER, RES_STREAM, TXT_STREAM, uint32_t, ubyte, ubyte_array> (res_file, txt_file);
			}
		}

#undef LANG_GEN
		return std::stringstream{};
	}

};