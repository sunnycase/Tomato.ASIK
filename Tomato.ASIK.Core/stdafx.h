// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once
#define NOMINMAX

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>
#include <mmreg.h>
#include <ppltasks.h>
#include <functional>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
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