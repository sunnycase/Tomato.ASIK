//
// Tomato ASIK
// Win32 异常
//
// (c) 2014 SunnyCase
// 创建日期：2014-12-19
#pragma once
#include "platform.h"

namespace Tomato
{
	class win32_exception
	{
	public:
		win32_exception(int error)
			:error(error)
		{

		}

		const string_t& what() const noexcept
		{
			return format_message(error);
		}

		int code() const noexcept
		{
			return error;
		}
	private:
		static string_t format_message(int error)
		{
			LPTSTR p_str = nullptr;
			auto len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&p_str, 0, nullptr);
			if (len)
			{
				string_t str(p_str, p_str + len);
				LocalFree(p_str);
				return str;
			}
			return string_t();
		}

		int error;
	};
}

#define THROW_WSA_IFNOT(val) { if(!(val)) throw ::Tomato::win32_exception(WSAGetLastError()); }
#define THROW_WIN_IFNOT(val) { if(!(val)) throw ::Tomato::win32_exception(GetLastError()); }
#define THROW_IF_FAILED(hr) { if(FAILED(hr)) throw ::Tomato::win32_exception(hr); }
#define THROW_IF_NOT(val, msg) {if(!(val)) throw std::exception(msg);}