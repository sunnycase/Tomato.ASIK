//
// Tomato ASIK
// 平台相关
//
// (c) 2014 SunnyCase
// 创建日期: 2014-12-19
#pragma once
#define NOMINMAX

#include <cstdint>
#include <cassert>
#ifdef _MANAGED
namespace Microsoft
{
	namespace WRL
	{
		template<typename T>
		class ComPtr;
	}
}
#else
#include <wrl.h>
#endif
#include <string>
#include <ppltasks.h>
#include <functional>

namespace wrl = Microsoft::WRL;
typedef std::wstring string_t;
typedef uint8_t byte;

#ifdef ASIKDLL
#define ASIKAPI _declspec(dllexport)
#else
#define ASIKAPI _declspec(dllimport)
#endif

#define ASIKCALL _stdcall