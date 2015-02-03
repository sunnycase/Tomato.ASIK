// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#define NOMINMAX

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>
#include <mmreg.h>
#include <ppltasks.h>
#include <functional>


// TODO:  在此处引用程序需要的其他头文件
#include <mfapi.h>
#include <Mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <amp_math.h>
#include <Strmif.h>
#include <Codecapi.h>
#include <atlcomcli.h>
#include <amp_graphics.h>

#include "../include/win32_exception.h"
#include "../include/mfhelpers.hpp"
#include "../vendors/ampfft/inc/amp_fft.h"