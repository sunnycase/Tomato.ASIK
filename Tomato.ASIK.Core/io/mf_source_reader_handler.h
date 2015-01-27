//
// Tomato ASIK
// MediaFoundtion 音源读取器回调
// 内部使用
//
// (c) 2014 SunnyCase
// 创建日期: 2014-12-19
#pragma once
#include "../../include/core/io/io.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

NSDEF_ASIK_CORE_IO

class mf_source_reader_handler : public wrl::RuntimeClass<
	wrl::RuntimeClassFlags<wrl::RuntimeClassType::ClassicCom>, IMFSourceReaderCallback>
{
public:
	mf_source_reader_handler();

	// 通过 RuntimeClass 继承
	STDMETHOD(OnReadSample)(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) override;
	STDMETHOD(OnFlush)(DWORD dwStreamIndex) override;
	STDMETHOD(OnEvent)(DWORD dwStreamIndex, IMFMediaEvent * pEvent) override;

	void set_current_read_event(concurrency::task_completion_event<wrl::ComPtr<IMFMediaBuffer>> ReadEvent);
private:
	concurrency::task_completion_event<wrl::ComPtr<IMFMediaBuffer>> currentReadEvent;
};

NSED_ASIK_CORE_IO