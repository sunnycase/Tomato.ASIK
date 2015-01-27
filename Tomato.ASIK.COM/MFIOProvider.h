// MFIOProvider.h : CMFIOProvider ������

#pragma once
#include "resource.h"       // ������



#include "TomatoASIKCOM_i.h"
#include "_IMFIOProviderEvents_CP.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif

using namespace ATL;


// CMFIOProvider

class ATL_NO_VTABLE CMFIOProvider :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMFIOProvider, &CLSID_MFIOProvider>,
	public IConnectionPointContainerImpl<CMFIOProvider>,
	public CProxy_IMFIOProviderEvents<CMFIOProvider>,
	public IDispatchImpl<IMFIOProvider, &IID_IMFIOProvider, &LIBID_TomatoASIKCOMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CMFIOProvider()
		:buffer(4096)
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_MFIOPROVIDER)


	BEGIN_COM_MAP(CMFIOProvider)
		COM_INTERFACE_ENTRY(IMFIOProvider)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
	END_COM_MAP()

	BEGIN_CONNECTION_POINT_MAP(CMFIOProvider)
		CONNECTION_POINT_ENTRY(__uuidof(_IMFIOProviderEvents))
	END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:



	STDMETHOD(Initialize)();
	STDMETHOD(LoadFile)(LPCWSTR fileName, DWORD* bufferSize);
	STDMETHOD(ReadAllSamples)(SAFEARRAY* buffer);
private:
	std::unique_ptr<Tomato::ASIK::Core::IO::io_provider> provider;
	Tomato::block_buffer<byte> buffer;
};

OBJECT_ENTRY_AUTO(__uuidof(MFIOProvider), CMFIOProvider)
