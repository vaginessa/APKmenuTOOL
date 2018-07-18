// dllcontext.h : Cdllcontext 的声明

#pragma once
#include "resource.h"       // 主符号

#include "dll.h"
#include "shlobj.h"
#include <list>
#include<atlstr.h>

#include "Function.h"

using namespace std;

#define SZ_MENUTEXT TEXT("安卓工具")

extern HINSTANCE  _hInstance;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif



// Cdllcontext

class ATL_NO_VTABLE Cdllcontext :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Cdllcontext, &CLSID_dllcontext>,
	public IDispatchImpl<Idllcontext, &IID_Idllcontext, &LIBID_dllLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
public:
	Cdllcontext()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DLLCONTEXT)


/*
vs2005编译需要添加预处理器：_ATL_NO_UUIDOF
ref:http://bbs.csdn.net/topics/30200356
Define the preprocessor symbol _ATL_NO_UUIDOF in the C/C++ tab of the Project Settings dialog box. 
This forces ATL to use the token pasting operator (##) rather than __uuidof() when forming the interface ID. 
*/
BEGIN_COM_MAP(Cdllcontext)
	COM_INTERFACE_ENTRY(Idllcontext)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		m_hBitmapInstall   = LoadImageFromRes(IDR_PNG_INSTALL, _hInstance, TEXT("PNG"));
		//m_hBitmapLogo    = LoadBitmap(_hInstance, MAKEINTRESOURCE(IDB_BITMAP2));
		//m_hBitmapLogo = LoadImageFromResource(IDR_PNG_LOGO,TEXT("PNG"));
		m_hBitmapLogo      = LoadImageFromRes(IDR_PNG_LOGO, _hInstance, TEXT("PNG"));
		m_hBitmapCopyPath  = LoadImageFromRes(IDB_PNG2,_hInstance,TEXT("PNG"));
		m_hBitmapDex2Jar   = LoadImageFromRes(IDR_PNG_DEX2JAR, _hInstance, TEXT("PNG"));
		m_hBitmapMa2Txt    = LoadImageFromRes(IDR_PNG_M2TXT, _hInstance, TEXT("PNG"));
		m_hBitmapApkinfo   = LoadImageFromRes(IDR_PNG_APKINFO, _hInstance, TEXT("PNG"));
		m_hBitmapSigninfo  = LoadImageFromRes(IDR_PNG_SIGNINFO, _hInstance, TEXT("PNG"));
		m_hBitmapDes       = LoadImageFromRes(IDR_PNG_DECOM, _hInstance, TEXT("PNG"));
		m_hBitmapUninstall = LoadImageFromRes(IDR_PNG_UNINSTALL, _hInstance, TEXT("PNG"));
		m_hBitmapSign      = LoadImageFromRes(IDR_PNG_SIGN, _hInstance, TEXT("PNG"));
		m_hBitmapCom       = LoadImageFromRes(IDR_PNG_COM, _hInstance, TEXT("PNG"));
		m_hBitmapAbout     = LoadImageFromRes(IDR_PNG_ABOUT, _hInstance, TEXT("PNG"));
		m_hBitmapDetect    = LoadImageFromRes(IDR_PNG_DETECT, _hInstance, TEXT("PNG"));
		m_hBitmapExtractIcon = LoadImageFromRes(IDR_PNG_EXTRACTICON, _hInstance, TEXT("PNG"));
		m_hBitmapAlign     = LoadImageFromRes(IDR_PNG_ALIGN, _hInstance, TEXT("PNG"));
		m_hBitmapPhoto     = LoadImageFromRes(IDR_PNG_PHOTO, _hInstance, TEXT("PNG"));
		m_hBitmapPhone     = LoadImageFromRes(IDR_PNG_PHONE, _hInstance, TEXT("PNG"));
		m_hBitmapCustom    = LoadImageFromRes(IDR_PNG_CUSTOM, _hInstance, TEXT("PNG"));
		m_hBitmapLib       = LoadImageFromRes(IDB_PNG1, _hInstance, TEXT("PNG"));
		m_hBitmapDeep      = LoadImageFromRes(IDB_PNG3, _hInstance, TEXT("PNG"));
		m_hBitmapRegular   = LoadImageFromRes(IDB_PNG4, _hInstance, TEXT("PNG"));
		return S_OK;
	}

	void FinalRelease()
	{
		if (m_hBitmapInstall)
		{
			DeleteObject(m_hBitmapInstall);
		}
		if (m_hBitmapLogo)
		{
			DeleteObject(m_hBitmapLogo);
		}
		if (m_hBitmapDex2Jar)
		{
			DeleteObject(m_hBitmapDex2Jar);
		}
		if (m_hBitmapMa2Txt)
		{
			DeleteObject(m_hBitmapMa2Txt);
		}
		if (m_hBitmapApkinfo)
		{
			DeleteObject(m_hBitmapApkinfo);
		}
		if (m_hBitmapSigninfo)
		{
			DeleteObject(m_hBitmapSigninfo);
		}
		if (m_hBitmapDes)
		{
			DeleteObject(m_hBitmapDes);
		}
		if (m_hBitmapUninstall)
		{
			DeleteObject(m_hBitmapUninstall);
		}
		if (m_hBitmapSign)
		{
			DeleteObject(m_hBitmapSign);
		}
		if (m_hBitmapCom)
		{
			DeleteObject(m_hBitmapCom);
		}
		if (m_hBitmapAbout)
		{
			DeleteObject(m_hBitmapAbout);
		}
		if(m_hBitmapDetect)
		{
			DeleteObject(m_hBitmapDetect);
		}
		if(m_hBitmapExtractIcon)
		{
			DeleteObject(m_hBitmapExtractIcon);
		}
		if(m_hBitmapAlign)
		{
			DeleteObject(m_hBitmapAlign);
		}
		if(m_hBitmapPhoto)
		{
			DeleteObject(m_hBitmapPhoto);
		}
		if(m_hBitmapPhone)
		{
			DeleteObject(m_hBitmapPhone);
		}
		if (m_hBitmapCustom)
		{
			DeleteObject(m_hBitmapCustom);
		}
	}

public:
	enum 
	{
		IDM_CTXMENU = 0,
	};
	HRESULT STDMETHODCALLTYPE Initialize( 
		/* [in] */ LPCITEMIDLIST pidlFolder,
		/* [in] */ IDataObject *pdtobj,
		/* [in] */ HKEY hkeyProgID);

	STDMETHOD(QueryContextMenu)(THIS_
		HMENU hmenu,
		UINT indexMenu,
		UINT idCmdFirst,
		UINT idCmdLast,
		UINT uFlags);

	STDMETHOD(InvokeCommand)(THIS_
		LPCMINVOKECOMMANDINFO lpici);

	STDMETHOD(GetCommandString)(THIS_
		UINT_PTR    idCmd,
		UINT        uType,
		UINT      * pwReserved,
		LPSTR       pszName,
		UINT        cchMax);

private:
	//TCHAR   m_pszFileName[MAX_PATH];
	list<CString> allFileName;
	HBITMAP m_hBitmapLogo;
	HBITMAP m_hBitmapCopyPath;
	HBITMAP m_hBitmapDex2Jar;
	HBITMAP m_hBitmapMa2Txt;
	HBITMAP m_hBitmapApkinfo;
	HBITMAP m_hBitmapSigninfo;
	HBITMAP m_hBitmapDes;
	HBITMAP m_hBitmapInstall;
	HBITMAP m_hBitmapUninstall;
	HBITMAP m_hBitmapSign;
	HBITMAP m_hBitmapCom;
	HBITMAP m_hBitmapAbout;
	HBITMAP m_hBitmapDetect;
	HBITMAP m_hBitmapExtractIcon;
	HBITMAP m_hBitmapAlign;
	HBITMAP m_hBitmapPhoto;
	HBITMAP m_hBitmapPhone;
	HBITMAP m_hBitmapCustom;
	HBITMAP m_hBitmapLib;
	HBITMAP m_hBitmapDeep;
	HBITMAP m_hBitmapRegular;
	list<CString> m_CustomMenu;

	CHAR    m_pszVerb[32];
	WCHAR   m_pwszVerb[32];

};

OBJECT_ENTRY_AUTO(__uuidof(dllcontext), Cdllcontext)
