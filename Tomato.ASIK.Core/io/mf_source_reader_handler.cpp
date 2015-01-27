//
// Tomato ASIK
// MediaFoundtion 音源读取器回调
// 内部使用
//
// (c) 2014 SunnyCase
// 创建日期: 2014-12-19
#include "stdafx.h"
#include "../stdafx.h"
#include "mf_source_reader_handler.h"

using namespace NS_ASIK_CORE_IO;
using namespace wrl;
using namespace concurrency;
using namespace Tomato;

mf_source_reader_handler::mf_source_reader_handler()
{
	currentReadEvent.set(nullptr);
}

STDMETHODIMP mf_source_reader_handler::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	try
	{
		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			THROW_IF_FAILED(MF_E_END_OF_STREAM);
		}
		else
		{
			THROW_IF_FAILED(hrStatus);
			if (!pSample) THROW_IF_FAILED(E_INVALIDARG);

			ComPtr<IMFMediaBuffer> mediaBuffer;
			BYTE* audioData = nullptr;
			DWORD audioDataLength = 0;

			// Since we are storing the raw byte data, convert this to a single buffer
			THROW_IF_FAILED(pSample->ConvertToContiguousBuffer(&mediaBuffer));
			currentReadEvent.set(mediaBuffer);
		}
	}
	catch (win32_exception& ex)
	{
		currentReadEvent.set_exception(ex);
		return ex.code();
	}
	catch (...)
	{
		currentReadEvent.set_exception(win32_exception(E_UNEXPECTED));
		return E_UNEXPECTED;
	}
	return S_OK;
}

STDMETHODIMP mf_source_reader_handler::OnFlush(DWORD dwStreamIndex)
{
	return S_OK;
}

STDMETHODIMP mf_source_reader_handler::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent)
{
	return S_OK;
}

void mf_source_reader_handler::set_current_read_event(task_completion_event<ComPtr<IMFMediaBuffer>> ReadEvent)
{
	// 上一个读取操作必须已经完成
	if (!currentReadEvent._IsTriggered())
		throw std::exception("last reading operation must be completed.");
	
	currentReadEvent = ReadEvent;
}
