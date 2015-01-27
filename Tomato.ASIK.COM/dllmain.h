// dllmain.h : 模块类的声明。

class CTomatoASIKCOMModule : public ATL::CAtlDllModuleT< CTomatoASIKCOMModule >
{
public :
	DECLARE_LIBID(LIBID_TomatoASIKCOMLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TOMATOASIKCOM, "{5DB89599-0EFE-469B-B954-F09F528AAA0D}")
};

extern class CTomatoASIKCOMModule _AtlModule;
