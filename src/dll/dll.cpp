// dll.cpp : DLL 导出的实现。


#include "stdafx.h"
#include "resource.h"
#include "dll.h"

HINSTANCE  _hInstance;
TCHAR szModulePath[MAX_PATH]; 
//NOTIFYICONDATA m_NotifyIconData;

class CdllModule : public CAtlDllModuleT< CdllModule >
{
public :
	DECLARE_LIBID(LIBID_dllLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DLL, "{FA46D921-04EF-4E0D-A7EA-95B46736D1DB}")
};

CdllModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL 入口点
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	_hInstance = hInstance;
	//Tips = FALSE;
	GetModuleFileName(hInstance, szModulePath, sizeof(szModulePath)/sizeof(TCHAR)); 
	//(_tcsrchr(szModulePath, _T('//')))[1] = 0; //删除文件名，只获得路径
	for (size_t i= wcslen(szModulePath)-1;i>=0;i--)
	{
		if (szModulePath[i] == _T('\\'))
		{
			szModulePath[i+1]=_T('t');
			szModulePath[i+2]=_T('o');
			szModulePath[i+3]=_T('o');
			szModulePath[i+4]=_T('l');
			szModulePath[i+5]=_T('s');
			szModulePath[i+6]=_T('\\');
			szModulePath[i+7]= 0;
			break;
		}
	}
	
	//m_notifyicondata.cbsize = (dword)sizeof(notifyicondata);
	//m_notifyicondata.hwnd = (hwnd)getmodulehandle(null);
	//m_notifyicondata.uid = null;
	//m_notifyicondata.uflags =  nif_info|nif_icon; // 设置托盘图标功能;
	////m_notifyicondata.ucallbackmessage = null; // 设置响应消息id;
	//m_notifyicondata.hicon = loadicon(null,idi_error);//loadicon(getmodulehandle(null), makeintresource(idi_icon1)); // 读取图标;
	//m_notifyicondata.dwinfoflags = niif_info;
	//shell_notifyicon(nim_add, &m_notifyicondata);
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// 用于确定 DLL 是否可由 OLE 卸载
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// 返回一个类工厂以创建所请求类型的对象
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - 将项添加到系统注册表
STDAPI DllRegisterServer(void)
{
    // 注册对象、类型库和类型库中的所有接口
	HRESULT hr;
	HKEY hKey;

	static char pszGUID[] = "{E13F371F-7B4C-4D04-AE25-98332A50BEEE}";
	hr = _AtlModule.DllRegisterServer();
	
	if (FAILED(hr))
	{
		return hr;
	}

	if (RegCreateKeyA(HKEY_CLASSES_ROOT, 
		"*\\shellex\\ContextMenuHandlers\\CtxMenu", &hKey) != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	if (RegSetValueA(hKey, NULL, REG_SZ, pszGUID,
		(DWORD)strlen(pszGUID)) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return E_FAIL;
	}
	//文件夹支持
	if (RegCreateKeyA(HKEY_CLASSES_ROOT, 
		"Directory\\shellex\\ContextMenuHandlers\\CtxMenu", &hKey) != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	if (RegSetValueA(hKey, NULL, REG_SZ, pszGUID,
		(DWORD)strlen(pszGUID)) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return E_FAIL;
	}
	return hr;
}


// DllUnregisterServer - 将项从系统注册表中移除
STDAPI DllUnregisterServer(void)
{
	RegDeleteKeyA(HKEY_CLASSES_ROOT, "*\\shellex\\ContextMenuHandlers\\CtxMenu");
	RegDeleteKeyA(HKEY_CLASSES_ROOT, "Directory\\shellex\\ContextMenuHandlers\\CtxMenu");

	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

