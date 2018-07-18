// dllcontext.cpp : Cdllcontext 的实现

#include "stdafx.h"
#include "dllcontext.h"
#ifndef _DEBUGPRINTF_H_ 
#define _DEBUGPRINTF_H_ 

#include<Windows.h> 
#include <tchar.h> 

//用于输出信息到编译器输出窗口的宏定义 
//使用win API，DEBUG版本会执行，RELEASE版本则不会 
//还可以使用DebugView，WinDbg等工具查看输出 

#ifdef _DEBUG 

#define DP0(fmt) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt));OutputDebugString(sOut);} 
#define DP1(fmt,var) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt),var);OutputDebugString(sOut);} 
#define DP2(fmt,var1,var2) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt),var1,var2);OutputDebugString(sOut);} 
#define DP3(fmt,var1,var2,var3) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt),var1,var2,var3);OutputDebugString(sOut);} 

#endif 

#ifndef _DEBUG 

#define DP0(fmt) ; 
#define DP1(fmt, var) ; 
#define DP2(fmt,var1,var2) ; 
#define DP3(fmt,var1,var2,var3) ; 

#endif 

#endif 

HRESULT STDMETHODCALLTYPE Cdllcontext::Initialize( 
	/* [in] */ LPCITEMIDLIST pidlFolder,
	/* [in] */ IDataObject *pdtobj,
	/* [in] */ HKEY hkeyProgID)
{
	//文件列表
	if( pdtobj != NULL ) 
	{
		STGMEDIUM stg = { TYMED_HGLOBAL };
		FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		//从pdtobj中获取文件列表指针,它们存储在HDROP格式中
		if( SUCCEEDED( pdtobj->GetData( &etc, &stg ) ) ) 
		{
			//取得HDROP句柄
			HDROP hDrop = (HDROP) ::GlobalLock( stg.hGlobal );
			//获取该操作中被选中的文件的数目
			UINT uCount = ::DragQueryFile( hDrop, (UINT) -1, NULL, 0 );
			allFileName.clear();
			for( UINT uIndex = 0; uIndex < uCount; uIndex++ ) 
			{
				TCHAR szFileName[MAX_PATH] = { 0 };
				CString strTemp = _T("");
				//获取文件名
				::DragQueryFile(hDrop, uIndex, szFileName, (sizeof(szFileName) / sizeof(TCHAR)) - 1);
				strTemp = szFileName;
				allFileName.push_back(strTemp);
				//szFileName为文件(夹)名
			}
			::GlobalUnlock( stg.hGlobal );
			::ReleaseStgMedium( &stg );
		}
	}    
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Cdllcontext::QueryContextMenu(THIS_
															 HMENU hmenu,
															 UINT indexMenu,
															 UINT idCmdFirst,
															 UINT idCmdLast,
															 UINT uFlags)
{
	//MENUITEMINFO mii;
	UINT uCmdID = idCmdFirst;
	uCmdID += 22;
	CString strConFileName, strCustomMenu;
	CString strSep = _T("-");

	static WCHAR szMenuText_Popup[] = L"安卓右键工具";
	static WCHAR szMenuText_0[] = L"复制路径";
	static WCHAR szMenuText_1[] = L"DEX->JAR";
	static WCHAR szMenuText_2[] = L"Manifest->TXT|AXML->XML";
	static WCHAR szMenuText_3[] = L"查看APK信息";
	static WCHAR szMenuText_4[] = L"查看签名信息";
	static WCHAR szMenuText_5[] = L"签名";
	static WCHAR szMenuText_6[] = L"安装（卸载安装）";
	static WCHAR szMenuText_7[] = L"安装（替换安装）";
	static WCHAR szMenuText_8[] = L"卸载";
	static WCHAR szMenuText_9[] = L"查看加壳信息";
	static WCHAR szMenuText_10[] = L"APK深度检测";
	static WCHAR szMenuText_11[] = L"查看手机信息";
	static WCHAR szMenuText_12[] = L"手机截图";
	static WCHAR szMenuText_13[] = L"提取图标";
	static WCHAR szMenuText_14[] = L"zipalign优化";
	static WCHAR szMenuText_15[] = L"反编译";
	static WCHAR szMenuText_16[] = L"回编译";
	static WCHAR szMenuText_17[] = L"提取so";
	static WCHAR szMenuText_18[] = L"lib规范性检查";
	static WCHAR szMenuText_19[] = L"自定义插件";
	static WCHAR szMenuText_20[] = L"关于 v1.2";

	strConFileName = szModulePath;
	strConFileName += _T("config\\config.ini"); 

	TCHAR szTmp[2048] = { 0 };

	GetPrivateProfileString(_T("CustomMenu"), _T("name"), _T(""), szTmp, 2048, strConFileName);
	strCustomMenu = szTmp;
	m_CustomMenu.clear();
	if (!strCustomMenu.IsEmpty())
	{
		SplitString(strCustomMenu, strSep, m_CustomMenu);
	}

	if (uFlags & CMF_DEFAULTONLY)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
	}


	InsertMenu(hmenu, indexMenu, MF_SEPARATOR | MF_BYPOSITION, 0, NULL); 

	indexMenu++;
	HMENU hSubMenu = CreateMenu();
	
	if(hSubMenu)
	{
		InsertMenu(hSubMenu, 0, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_0);
		SetMenuItemBitmaps(hSubMenu, 0, MF_BYPOSITION, m_hBitmapCopyPath, m_hBitmapCopyPath);
		InsertMenu(hSubMenu, 1, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_1);
		SetMenuItemBitmaps(hSubMenu, 1, MF_BYPOSITION, m_hBitmapDex2Jar, m_hBitmapDex2Jar);//dex->jar
		InsertMenu(hSubMenu, 2, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_2);
		SetMenuItemBitmaps(hSubMenu, 2, MF_BYPOSITION, m_hBitmapMa2Txt, m_hBitmapMa2Txt);//manifest->txt
		InsertMenu(hSubMenu, 3, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_3);
		SetMenuItemBitmaps(hSubMenu, 3, MF_BYPOSITION, m_hBitmapApkinfo, m_hBitmapApkinfo);//apk info
		InsertMenu(hSubMenu, 4, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_4);
		SetMenuItemBitmaps(hSubMenu, 4, MF_BYPOSITION, m_hBitmapSigninfo, m_hBitmapSigninfo);//签名信息
		InsertMenu(hSubMenu, 5, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_5);
		SetMenuItemBitmaps(hSubMenu, 5, MF_BYPOSITION, m_hBitmapSign, m_hBitmapSign);//签名
		InsertMenu(hSubMenu, 6, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_6);
		SetMenuItemBitmaps(hSubMenu, 6, MF_BYPOSITION, m_hBitmapInstall, m_hBitmapInstall);//卸载安装
		InsertMenu(hSubMenu, 7, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_7);
		SetMenuItemBitmaps(hSubMenu, 7, MF_BYPOSITION, m_hBitmapInstall, m_hBitmapInstall);//替换安装
		InsertMenu(hSubMenu, 8, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_8);
		SetMenuItemBitmaps(hSubMenu, 8, MF_BYPOSITION, m_hBitmapUninstall, m_hBitmapUninstall);//卸载
		InsertMenu(hSubMenu, 9, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_9);
		SetMenuItemBitmaps(hSubMenu, 9, MF_BYPOSITION, m_hBitmapDetect, m_hBitmapDetect);//加壳信息

		InsertMenu(hSubMenu, 10, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_10);//深度检测
		SetMenuItemBitmaps(hSubMenu, 10, MF_BYPOSITION, m_hBitmapDeep, m_hBitmapDeep);


		InsertMenu(hSubMenu, 11, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_11);
		SetMenuItemBitmaps(hSubMenu, 11, MF_BYPOSITION, m_hBitmapPhone, m_hBitmapPhone);//手机信息
		InsertMenu(hSubMenu, 12, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_12);
		SetMenuItemBitmaps(hSubMenu, 12, MF_BYPOSITION, m_hBitmapPhoto, m_hBitmapPhoto);//截图
		InsertMenu(hSubMenu, 13, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_13);
		SetMenuItemBitmaps(hSubMenu, 13, MF_BYPOSITION, m_hBitmapExtractIcon, m_hBitmapExtractIcon);//提取图标
		InsertMenu(hSubMenu, 14, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_14);
		SetMenuItemBitmaps(hSubMenu, 14, MF_BYPOSITION, m_hBitmapAlign, m_hBitmapAlign);//优化
		InsertMenu(hSubMenu, 15, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_15);
		SetMenuItemBitmaps(hSubMenu, 15, MF_BYPOSITION, m_hBitmapDes, m_hBitmapDes);//反编译
		InsertMenu(hSubMenu, 16, MF_STRING  | MF_BYPOSITION, uCmdID++, szMenuText_16);
		SetMenuItemBitmaps(hSubMenu, 16, MF_BYPOSITION, m_hBitmapCom, m_hBitmapCom);//回编译
		InsertMenu(hSubMenu, 17, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_17);
		SetMenuItemBitmaps(hSubMenu, 17, MF_BYPOSITION, m_hBitmapLib, m_hBitmapLib);//提取So

		InsertMenu(hSubMenu, 18, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_18);
		SetMenuItemBitmaps(hSubMenu, 18, MF_BYPOSITION, m_hBitmapRegular, m_hBitmapRegular);//lib规范性检查
		HMENU hCustomSubMenu = NULL;
		if (m_CustomMenu.size() > 0){}
			hCustomSubMenu = CreateMenu();
		
		if (hCustomSubMenu)
		{
			int i = 0;
			for (list<CString>::iterator it = m_CustomMenu.begin(); it != m_CustomMenu.end(); ++it, i++)
			{
				InsertMenu(hCustomSubMenu, i, MF_STRING | MF_BYPOSITION, uCmdID++, *it);
			}

			InsertMenu(hSubMenu, 19, MF_STRING | MF_POPUP | MF_BYPOSITION, (UINT_PTR)hCustomSubMenu, szMenuText_19);
			SetMenuItemBitmaps(hSubMenu, 19, MF_BYPOSITION, m_hBitmapCustom, m_hBitmapCustom);//自定义插件

			InsertMenu(hSubMenu, 20, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_20);
			SetMenuItemBitmaps(hSubMenu, 20, MF_BYPOSITION, m_hBitmapAbout, m_hBitmapAbout);//关于
		}
		else
		{
			InsertMenu(hSubMenu, 19, MF_STRING | MF_BYPOSITION, uCmdID++, szMenuText_20);
			SetMenuItemBitmaps(hSubMenu, 19, MF_BYPOSITION, m_hBitmapAbout, m_hBitmapAbout);
		}
	}
	InsertMenu(hmenu, indexMenu, MF_STRING | MF_POPUP | MF_BYPOSITION, (UINT_PTR)hSubMenu, szMenuText_Popup);
	SetMenuItemBitmaps(hmenu, indexMenu, MF_BYPOSITION, m_hBitmapLogo, m_hBitmapLogo);
	indexMenu++;
	InsertMenu(hmenu, indexMenu, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
	indexMenu++;

	lstrcpynA(m_pszVerb, "protected_run", 32);
	lstrcpynW(m_pwszVerb, L"protected_run", 32);

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, uCmdID-idCmdFirst);
}

HRESULT STDMETHODCALLTYPE Cdllcontext::InvokeCommand(THIS_
														LPCMINVOKECOMMANDINFO lpici)
{
	BOOL fEx = FALSE;
	BOOL fUnicode = FALSE;

	if(lpici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
	{
		fEx = TRUE;
		if((lpici->fMask & CMIC_MASK_UNICODE))
		{
			fUnicode = TRUE;
		}
	}

	if( !fUnicode && HIWORD(lpici->lpVerb))
	{
		if(StrCmpIA(lpici->lpVerb, m_pszVerb))
		{
			return E_FAIL;
		}
	}

	else if( fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX *) lpici)->lpVerbW))
	{
		if (StrCmpIW(((CMINVOKECOMMANDINFOEX *)lpici)->lpVerbW, m_pwszVerb))
		{
			return E_FAIL;
		}
		
	}
	else
	{
		switch ( LOWORD( lpici->lpVerb ))
		{
		case 22:
			{
			CString filePath("");
			size_t index = allFileName.size();

			//单文件处理
			if (index == 1)
			{
				
				filePath.Append(allFileName.front());
			}
			else
			{
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
				{
				filePath.Append(*itfile);
				filePath.Append(_T("\r\n"));
				}
			filePath.Delete(filePath.GetLength() - 2, 2);
			}
			HANDLE hThread;
			ThreadData* plugData = new ThreadData;
			plugData->strFilePath = filePath;
			hThread = (HANDLE)_beginthreadex(NULL, 0, GetCopyPath, (void*)plugData, 0, NULL);
			CloseHandle(hThread);
			allFileName.clear();
			break;
			}
		case 23:
			{
			//MessageBox(NULL, _T("test1"), NULL, 0);
			//Dex->jar	
				for(list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)  
				{
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = *itfile;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadDex2Jar, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
				}

				allFileName.clear();

				break;
			}
		case 24:
			{
			//MessageBox(NULL, _T("test2"), NULL, 0);
			//manifest->txt
				size_t index = allFileName.size();

				//单文件处理
				if (index==1)
				{
					//CString szFile;
					//szFile = allFileName.pop_front();
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = allFileName.front();
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadMani2TxtSignal, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
					allFileName.clear();
					break;
				}
				//批量处理
				for(list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)  
				{
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = *itfile;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadMani2Txt, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
				}
				allFileName.clear();
				break;
			}
		case 25:
			{
			//MessageBox(NULL, _T("test3"), NULL, 0);
				//查看apk信息
				for(list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)  
				{
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = *itfile;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadApkInfor, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
				}
				
				allFileName.clear();
				break;
			}
		case 26:
			{
			//MessageBox(NULL, _T("test4"), NULL, 0);
				//获取签名信息
				for(list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)  
				{
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = *itfile;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadGetSigninfo, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
				}

				allFileName.clear();
				break;
			}
		case 27:
			{
			//签名
			//MessageBox(NULL, _T("test5"), NULL, 0);
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadApkSign, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}
			allFileName.clear();
			break;
			}
		case 28:
			{
			//安装（卸载安装）
			//MessageBox(NULL, _T("test6"), NULL, 0);
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadApkInstall, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();

			break;
			
			}
		case 29:
			{
			//MessageBox(NULL, _T("test7"), NULL, 0);
			//安装（替换安装）
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadApkInstallfore, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();

			break;
			
			}
		case 30:
			{
			//MessageBox(NULL, _T("test8"), NULL, 0);
			//卸载
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadApkUninstall, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
			
			}
		case 31:
			{
			//MessageBox(NULL, _T("test9"), NULL, 0);
			//获取加壳信息
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadGetWrapperInfo, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
			}
		case 32:
		{
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadDeepDetect, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
		}
		case 33:
			{
			//MessageBox(NULL, _T("test10"), NULL, 0);
			//获取手机信息
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadGetPhoneInfo, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
			}
		case 34:
			{
			//MessageBox(NULL, _T("test11"), NULL, 0);
			//手机截图
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				//MessageBox(NULL,_T("手机截图"),NULL,0);
				HANDLE hThread;
				ThreadData *plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadScreenCap, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
			}
		case 35:
			{
			//MessageBox(NULL, _T("test12"), NULL, 0);
			//提取图标
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadGetExtraIcon, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
			}
		case 36:
			{
			//MessageBox(NULL, _T("test13"), NULL, 0);
			//zipalign优化
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				//MessageBox(NULL,_T("zipalign优化"),NULL,0);
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadApkAlign, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
			}
		case 37:
			{
			//MessageBox(NULL, _T("test14"), NULL, 0);
				//反编译
				for(list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)  
				{
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = *itfile;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadDeApk, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
				}

				allFileName.clear();
				break;
			}
		case 38:
			{
			//MessageBox(NULL, _T("test15"), NULL, 0);
				//回编译
				for(list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)  
				{
					HANDLE hThread;
					ThreadData* plugData = new ThreadData;
					plugData->strFilePath = *itfile;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadComApk, (void*)plugData, 0, NULL);
					CloseHandle(hThread);
				}

				allFileName.clear();
				break;
			}
		case 39:
		{
			//提取LIB
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadGetExtraLib, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
		}
		case 40:
		{
			//lib规范性检查
			for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
			{
				HANDLE hThread;
				ThreadData* plugData = new ThreadData;
				plugData->strFilePath = *itfile;
				hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadIsRegular, (void*)plugData, 0, NULL);
				CloseHandle(hThread);
			}

			allFileName.clear();
			break;
		}
		default:
			{
			//MessageBox(NULL, _T("test16"), NULL, 0);
			if (LOWORD(lpici->lpVerb) - 41 == m_CustomMenu.size())
				{
					//关于
					HANDLE hThread;
					hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadAbout, NULL, 0, NULL);
					CloseHandle(hThread);
					allFileName.clear();
					break;
				}
				else if (LOWORD(lpici->lpVerb) > 40 && LOWORD(lpici->lpVerb) <= m_CustomMenu.size() + 40)
				{
					CString strIndex;
					strIndex.Format(_T("%d"), LOWORD(lpici->lpVerb) - 40);
					//自定义插件
					for (list<CString>::iterator itfile = allFileName.begin(); itfile != allFileName.end(); ++itfile)
					{
						HANDLE hThread;
						ThreadData* plugData = new ThreadData;
						plugData->strFilePath = *itfile;
						plugData->strPlugIndex = strIndex;
						
						hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadCustomPlugFunc, (void*)plugData, 0, NULL);
						CloseHandle(hThread);
					}
					allFileName.clear();
					break;
				}
				return E_INVALIDARG;
				break;
			}
		}

		//在此处理点击事件.
		return S_OK;

	}

	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE Cdllcontext::GetCommandString(THIS_
															 UINT_PTR    idCmd,
															 UINT        uType,
															 UINT      * pwReserved,
															 LPSTR       pszName,
															 UINT        cchMax)
{
	USES_CONVERSION;
	LPCTSTR szPrompt;
	if ( uType & GCS_HELPTEXT )
	{
		switch ( idCmd )
		{
		case 0:
			szPrompt = _T("复制路径");
			break;
		case 1:
			szPrompt = _T("DEX->JAR");
			break;
		case 2:
			szPrompt = _T("manifest->txt");
			break;
		/*case 2:
			szPrompt = _T("AXML->XML");*/
		case 3:
			szPrompt = _T("查看apk信息");
			break;
		case 4:
			szPrompt = _T("查看签名信息");
			break;
		case 5:
			szPrompt = _T("签名");
			
			break;
		case 6:
			szPrompt = _T("卸载安装");
			
			break;
		case 7:
			szPrompt = _T("替换安装");
			
			break;
		case 8:
			szPrompt = _T("卸载");
			
			break;
		case 9:
			szPrompt = _T("查看加壳信息");
			break;
		case 10:
			szPrompt = _T("APK深度检测");
			
			break;
		case 11:
			szPrompt = _T("查看手机信息");
			break;
		case 12:
			szPrompt = _T("手机截图");
			break;
		case 13:
			szPrompt = _T("提取图标");
			break;
		case 14:
			szPrompt = _T("zipalign优化");
			break;
		case 15:
			szPrompt = _T("反编译");
			break;
		case 16:
			szPrompt = _T("回编译");
			break;
		case 17:
			szPrompt = _T("提取so");
			break;
		case 18:
			szPrompt = _T("lib规范性检查");
			break;
		default:
			ATLASSERT(0);           // should never get here
			if (idCmd - 19 == m_CustomMenu.size())
			{
				szPrompt = _T("关于 v1.2");
			}
			else if (idCmd > 18 && idCmd <= m_CustomMenu.size() + 18)
			{
				szPrompt = _T("自定义插件");
				break;
			}
			return E_INVALIDARG;
			break;
		}      

		if ( uType & GCS_UNICODE )
		{
			 //我们需要将 pszName 转化为一个 Unicode 字符串, 接着使用Unicode字符串拷贝 API.
			lstrcpynW ( (LPWSTR) pszName, T2CW(szPrompt), cchMax );
		}
		else
		{
			 //使用 ANSI 字符串拷贝API 来返回帮助字符串.
			lstrcpynA ( pszName, T2CA(szPrompt), cchMax );
		}
		return S_OK;
	}
	return E_INVALIDARG;
}

