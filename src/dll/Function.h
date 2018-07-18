#include "ZipFunction.h"
#include<atlconv.h>
#include <atlimage.h>
#include <Strsafe.h>
#include <vector>
#include <list>
#include <io.h>
#include <Gdiplus.h>

using namespace Gdiplus;

#pragma comment(lib,"gdiplus") 

extern TCHAR szModulePath[MAX_PATH]; 
extern HINSTANCE  _hInstance;
//extern NOTIFYICONDATA m_NotifyIconData;
typedef struct _ThreadData
{
	CString strFilePath;
	CString strPlugIndex;
}ThreadData;


int SplitString(const CString&strSrc, const CString&strSep, list<CString>&ltStr)
{
	ltStr.clear();
	TCHAR *pContext = NULL;

	TCHAR *pToken = _tcstok_s((LPTSTR)(LPCTSTR)strSrc, (LPCTSTR)strSep, &pContext);
	while (pToken != NULL) {
		ltStr.push_back((CString)pToken);
		pToken = _tcstok_s(NULL, (LPCTSTR)strSep, &pContext);
	}

	return (int)ltStr.size();
}

//利用Gdiplus从资源加载png图片到hbitmap
HBITMAP LoadImageFromRes(UINT pResourceID, HMODULE hInstance, LPCTSTR pResourceType/* = "PNG"*/)
{
	HBITMAP hBitmap = NULL;
	LPCTSTR pResourceName = MAKEINTRESOURCE(pResourceID);
	ULONG_PTR gdiplusToken;


	HRSRC hResource = FindResource(hInstance, pResourceName, pResourceType);

	if(!hResource)
	{
		
		return NULL;
	}

	DWORD dwResourceSize = SizeofResource(hInstance, hResource);

	if(!dwResourceSize)
	{
		
		return NULL;
	}

	const void* pResourceData = LockResource(LoadResource(hInstance, hResource));

	if(!pResourceData)
	{
		
		return NULL;
	}

	HGLOBAL hResourceBuffer = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);

	if(!hResourceBuffer)
	{
		GlobalFree(hResourceBuffer);
		
		return NULL;
	}

	void* pResourceBuffer = GlobalLock(hResourceBuffer);

	if(!pResourceBuffer)
	{
		GlobalUnlock(hResourceBuffer);
		GlobalFree(hResourceBuffer);
		
		return NULL;
	}

	CopyMemory(pResourceBuffer, pResourceData, dwResourceSize);
	IStream* pIStream = NULL;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput; 
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if(CreateStreamOnHGlobal(hResourceBuffer, FALSE, &pIStream)==S_OK)
	{
		//<strong>//  返回空指针</strong>
		Gdiplus::Image *pImage = Gdiplus::Image::FromStream(pIStream);
		Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromStream(pIStream);

		pIStream->Release();
		GlobalUnlock(hResourceBuffer);
		GlobalFree(hResourceBuffer);

		if(pBitmap==NULL)
		{
			return NULL;
		}

		Gdiplus::Status result = pBitmap->GetLastStatus();

		if(result==Gdiplus::Ok)
		{
			pBitmap->GetHBITMAP(NULL, &hBitmap);
			return hBitmap;
		}

		delete pBitmap;
		//throw getGdiplusErrorString(result);
	}
	GlobalUnlock(hResourceBuffer);
	GlobalFree(hResourceBuffer);
	Gdiplus::GdiplusShutdown(gdiplusToken);

	return NULL;
}

//该函数在dll中图像显示花屏
HBITMAP LoadImageFromResource(UINT nResourceId, LPCTSTR pszResourceName/* = "PNG"*/)
{
	HBITMAP hBitmap = NULL;
	HINSTANCE hInstance = _hInstance;
	HRSRC hRsrc = ::FindResource (hInstance, MAKEINTRESOURCE(nResourceId), pszResourceName); // type
	if ( hRsrc==NULL ){
		return hBitmap;
	}

	// load resource into memory
	DWORD len = SizeofResource(hInstance, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInstance, hRsrc);
	if ( lpRsrc==NULL ){
		return hBitmap;
	}

	// Allocate global memory on which to create stream
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem,lpRsrc,len);
	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem,FALSE,&pstm);

	// load from stream
	CImage image;
	image.Load(pstm);
	hBitmap = image.Detach();
	// free/release stuff
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);

	return hBitmap;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders  
	UINT  size = 0;         // size of the image encoder array in bytes  

	ImageCodecInfo* pImageCodecInfo = NULL;

	//2.获取GDI+支持的图像格式编码器种类数以及ImageCodecInfo数组的存放大小  
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure  

					//3.为ImageCodecInfo数组分配足额空间  
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure  

					//4.获取所有的图像编码器信息  
	GetImageEncoders(num, size, pImageCodecInfo);

	//5.查找符合的图像编码器的Clsid  
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success  
		}
	}

	//6.释放步骤3分配的内存  
	free(pImageCodecInfo);
	return -1;  // Failure  
}

CString ScalePng(CString strFileName, float scalex, float scaley)
{
	ULONG_PTR token;
	CLSID   clsid;
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup(&token, &input, NULL);

	CString strBmp = strFileName;
	strBmp.Replace(_T(".png"), _T("_bak.png"));

	GetEncoderClsid(L"image/png", &clsid);
	if (clsid.Data1 != 0 || clsid.Data2 != 0 || clsid.Data3 != 3)
	{
		//MessageBox(NULL, _T("image/png"), NULL, 0);
		Gdiplus::Bitmap image(strFileName);
		if (image.GetLastStatus() == Gdiplus::Ok)
		{
			UINT srcWidth = image.GetWidth();
			UINT srcHeight = image.GetHeight();
			float dstWidth = srcWidth * scalex;
			float dstHeight = srcHeight * scaley;
			Gdiplus::Bitmap newImage((u_int)dstWidth, (u_int)dstHeight);
			Gdiplus::Graphics graphics(&newImage);
			graphics.DrawImage(&image, 0u, 0u, dstWidth, dstHeight);
			newImage.Save(strBmp, &clsid);
		}
	}

	Gdiplus::GdiplusShutdown(token);

	return strBmp;
}

/*
png-->bmp，
成功则返回TRUE, strOutPut保存转换后图片路径
失败则返回FALSE, strOutPut返回出错原因
*/
BOOL ConvertPic(CString strFile, OUT CString& strOutPut)
{
	BOOL bRet = TRUE;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	strOutPut = strFile;
	strOutPut.Replace(_T(".png"), _T(".bmp"));

	//1.初始化GDI+，以便后续的GDI+函数可以成功调用  
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Status  stat;

	//7.创建Image对象并加载图片  
	Image*   image = new Image(strFile);

	// Get the CLSID of the PNG encoder.  
	GetEncoderClsid(L"image/bmp", &encoderClsid);

	//8.调用Image.Save方法进行图片格式转换，并把步骤3)得到的图像编码器Clsid传递给它  
	stat = image->Save(strOutPut, &encoderClsid, NULL);

	if (stat != Ok)
	{
		bRet = FALSE;
		strOutPut.Format(_T("转换png到bmp失败: stat = %d/n"), stat);
	}

	//9.释放Image对象  
	delete image;
	//10.清理所有GDI+资源  
	GdiplusShutdown(gdiplusToken);
	return bRet;
}

BOOL SetPngToClipboard(CString strPngPath, OUT CString& strOutput)
{
	BOOL bRet = TRUE;
	CString strBmpPath;
	if (ConvertPic(strPngPath, strBmpPath) == FALSE)
	{
		strOutput = strBmpPath;
		DeleteFile(strPngPath);
		return FALSE;
	}
	//MessageBox(NULL, strBmpPath, NULL, 0);
	HBITMAP  hBmp = (HBITMAP)LoadImage(0, strBmpPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	if (hBmp == NULL)
	{
		strOutput = _T("LoadImage加载bmp图像失败");
		DeleteFile(strPngPath);
		DeleteFile(strBmpPath);
		return FALSE;
	}

	BITMAP bmp;

	GetObject(hBmp, sizeof(BITMAP), &bmp);
	HDC hcmdc1 = CreateCompatibleDC(NULL);
	HBITMAP hOldMap1 = (HBITMAP)SelectObject(hcmdc1, hBmp);

	HDC hcmdc2 = CreateCompatibleDC(NULL);

	HBITMAP hBitMap2 = ::CreateCompatibleBitmap(hcmdc1, bmp.bmWidth, bmp.bmHeight);
	HBITMAP hOldMap2 = (HBITMAP)SelectObject(hcmdc2, hBitMap2);
	StretchBlt(hcmdc2, 0, 0, bmp.bmWidth, bmp.bmHeight, hcmdc1, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

	CImage image;
	image.Attach(hBitMap2);
	//CBitmap *
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, image);
		CloseClipboard();
	}
	else
	{
		strOutput = _T("打开剪贴板失败");
		bRet = FALSE;
	}

	image.Detach();
	::SelectObject(hcmdc1, hOldMap1);
	::SelectObject(hcmdc2, hOldMap2);
	::DeleteObject(hBitMap2);

	::DeleteDC(hcmdc1);
	::DeleteDC(hcmdc2);
	::DeleteObject(hBmp);

	DeleteFile(strPngPath);
	DeleteFile(strBmpPath);
	return bRet;
}

void ShowTips(CString szTitle,CString szTips)
{
	//MessageBox(NULL, szTips, NULL, 0);
	NOTIFYICONDATA m_NotifyIconData;
	m_NotifyIconData.cbSize  = (DWORD)sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd    = (HWND) GetModuleHandle(NULL);
	m_NotifyIconData.uID     = NULL;
	m_NotifyIconData.uFlags  = NIF_MESSAGE|NIF_TIP|NIF_INFO; // 设置托盘图标功能;
	m_NotifyIconData.uCallbackMessage = NULL; // 设置响应消息ID;
	m_NotifyIconData.hIcon   = NULL;//LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)); // 读取图标;
	Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);// 添加托盘图标;
	m_NotifyIconData.dwInfoFlags = NIIF_INFO; 
	StringCchCopy(m_NotifyIconData.szInfoTitle, _countof(m_NotifyIconData.szInfoTitle),szTitle);
	StringCchCopy(m_NotifyIconData.szInfo, _countof(m_NotifyIconData.szInfo),szTips);
	m_NotifyIconData.uTimeout = 700; // 超时时间
	Shell_NotifyIcon(NIM_MODIFY,&m_NotifyIconData);
}

//执行命令函数
BOOL RunCmdFile(LPCTSTR lpstrCmd)  
{  
	PROCESS_INFORMATION pi;   
	STARTUPINFO si;   
	ZeroMemory(&si,sizeof(si));   
	si.cb = sizeof(si);  
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);   
	if(!CreateProcess(NULL,  
		(LPTSTR)lpstrCmd,  
		NULL,  
		NULL,  
		FALSE,  
		0,  
		NULL,  
		NULL,  
		&si,&pi))   
	{   
		CString cs = _T("");
		//cs.Format(_T("%s"), (GetLastError()));
		MessageBox(NULL,_T("创建线程失败"),NULL,MB_OK); 
		return FALSE;
	}   
	else   
	{   
		WaitForSingleObject(pi.hProcess,INFINITE); 
		CloseHandle(pi.hProcess);   
		CloseHandle(pi.hThread);   
	}   
	return TRUE;  
}  
BOOL WriteLogFile(CString strFilePath,const CStringA& strFileContent)
{
	DWORD dwReadWrite = 0;
	HANDLE hFile = CreateFile(strFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ) {
		WriteFile(hFile, strFileContent, strFileContent.GetLength(), &dwReadWrite, NULL);
		CloseHandle(hFile);
		return TRUE;
	}else{
		_tprintf_s(_T("create tempbat.bat file failed, error code: %d\r\n"), GetLastError());
		return FALSE;
	}
}

//执行命令并写log信息
BOOL RunCmdAndWriteFile(LPCTSTR szCmdLine,CString strFile)  
{  
	//DWORD iReturned;
	SECURITY_ATTRIBUTES sa={sizeof(sa),NULL,TRUE};  
	SECURITY_ATTRIBUTES *psa=NULL;  
	DWORD dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE;  
	OSVERSIONINFO osVersion={0};  
	osVersion.dwOSVersionInfoSize =sizeof(osVersion);  
	if (GetVersionEx(&osVersion))  
	{  
		if ( osVersion.dwPlatformId ==VER_PLATFORM_WIN32_NT )  
		{  
			psa=&sa;  
			dwShareMode|=FILE_SHARE_DELETE;  
		}  
	}  
	//根据版本设置共享模式和安全属性  
	HANDLE hConsoleRedirect=CreateFile (  
		strFile,  
		GENERIC_WRITE|GENERIC_READ,  
		dwShareMode,  
		psa,  
		OPEN_ALWAYS,  
		FILE_ATTRIBUTE_NORMAL,  
		NULL);  
	if (hConsoleRedirect == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL,_T("创建文件失败!"),NULL,MB_OK); 
		return FALSE;
	}


	PROCESS_INFORMATION pi={0};   
	STARTUPINFO si;   
	ZeroMemory(&si,sizeof(si));   
	si.cb = sizeof(si);  
	si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);   
	si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput =hConsoleRedirect; 
	si.wShowWindow = SW_HIDE;
	
	if(!CreateProcess(NULL,(LPTSTR)szCmdLine,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi))   
	{   
		MessageBox(NULL,_T("不能创建进程"),NULL,MB_OK); 
	}   
	else   
	{   
		WaitForSingleObject(pi.hProcess,INFINITE);
		
		CloseHandle(pi.hProcess);   
		CloseHandle(pi.hThread);   
	}   



	CloseHandle(hConsoleRedirect);  
	return TRUE;  
} 

CStringA RunCmdAndOutput( LPCTSTR lpszCmdLine, DWORD dwMilliseconds = INFINITE )
{
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.bInheritHandle = true; 
	sa.lpSecurityDescriptor = NULL; 
	HANDLE hReadPipe, hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, NULL)) 
		return "Create pipe failed!";

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.dwFlags = STARTF_USESHOWWINDOW |  STARTF_USESTDHANDLES; 
	si.wShowWindow = SW_HIDE;
	if (CreateProcess(NULL, (LPTSTR)(LPCTSTR)lpszCmdLine, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi) == 0)
	{
		DWORD dwError = GetLastError();
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		return "CreateProcess failed!";
	}
	CloseHandle(hWritePipe);

	const int BUF_LEN = 1024;
	char buffer[BUF_LEN + 1] = {0}; 
	DWORD dwRead = 0; 
	CStringA strLog;
	while (ReadFile(hReadPipe, buffer, BUF_LEN*sizeof(BYTE) , &dwRead, NULL) != NULL)
	{ 
		strLog.AppendFormat("%s", buffer);
		RtlZeroMemory(buffer, BUF_LEN);
	}
	CloseHandle(hReadPipe);
	DWORD ret = WaitForSingleObject(pi.hProcess, dwMilliseconds);
	if (ret == WAIT_TIMEOUT)
		TerminateProcess(pi.hProcess, 0);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return strLog;
}

void GetApkFileInfo(CString strFilePath)
{
	HANDLE hFile;//文件句柄
	int fileSize = 0;
	int wcs_len = 0;
	int i = 0;
	DWORD reSize = 0;
	CString str;
	CHAR *pBuf = NULL;
	TCHAR *pPtr = NULL;
	CString strReturn;
	CString strContent;
	CString strTemp;
	int index = -1;

	hFile=CreateFile(
		strFilePath,					
		GENERIC_READ|GENERIC_WRITE,			
		FILE_SHARE_READ,						
		NULL,					
		OPEN_EXISTING,			
		NULL,	
		NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		return;
	}

	fileSize = GetFileSize(hFile,NULL);
	pBuf = new CHAR[fileSize+1];

	ReadFile(hFile,pBuf,fileSize,&reSize,NULL);
	pBuf[fileSize]='\0';

	wcs_len=MultiByteToWideChar(CP_UTF8,0,pBuf,-1,NULL,NULL);
	pPtr=new TCHAR[wcs_len + 1];  //分配内存空间
	ZeroMemory(pPtr, (wcs_len+1) * sizeof(TCHAR));
	if(pPtr!=NULL)
	{ 
		//将char字串转换为wchar_t字串
		MultiByteToWideChar(CP_UTF8,0,pBuf,(int)strlen(pBuf),pPtr,wcs_len);
	}
	if (pBuf != NULL)
	{
		delete [] pBuf;
		pBuf = NULL;
	}
	strReturn = _T("  软件名称: ");
	//应用软件名称：
	strContent.Format(_T("%s"),pPtr);
	strTemp = strContent;
	index = strContent.Find(_T("application-label:"));
	strContent.Delete(0,index+19);
	index = strContent.Find(_T('\''));
	strContent = strContent.Left(index);

	strReturn += (strContent + _T("\r\n"));
	//软件包名：
	strContent = strTemp;
	index = strContent.Find(_T("package: name="));
	strContent.Delete(0,index+15);
	index = strContent.Find(_T('\''));
	strContent = strContent.Left(index);

	strReturn += _T("  软件包名: ");
	strReturn += (strContent + _T("\r\n"));

	//软件版本：
	strContent = strTemp;
	index = strContent.Find(_T("versionName="));
	strContent.Delete(0,index+13);
	index = strContent.Find(_T('\''));
	strContent = strContent.Left(index);

	strReturn += _T("  软件版本: ");
	strReturn += (strContent + _T("\r\n"));

	//启动Activity：
	strContent = strTemp;
	index = strContent.Find(_T("launchable-activity: name="));
	strContent.Delete(0,index+27);
	index = strContent.Find(_T('\''));
	strContent = strContent.Left(index);

	strReturn += _T("  启动Activity: ");
	strReturn += (strContent + _T("\r\n"));

	//启动权限:
	strContent = strTemp;
	if (strContent.Find(_T("uses-permission:")) != -1)
	{
		strReturn += _T("  所需权限：");
	}
	while ((strContent.Find(_T("uses-permission:"))) != -1)
	{
		if (i != 0)
		{
			strReturn += _T("                  ");
		}
		index = strContent.Find(_T("uses-permission:"));
		strContent.Delete(0, index + 23);
		index = strContent.Find(_T("\'"));
		CString temp = strContent.Left(index);
		strReturn += temp + _T("\r\n");
		strContent.Delete(0, index);
		i++;
	}
	SetFilePointer (hFile, 0, NULL, FILE_BEGIN);
	SetEndOfFile(hFile);
	
	wcs_len = WideCharToMultiByte(CP_ACP,0,strReturn,-1,NULL,0,NULL,NULL);
	pBuf = new CHAR[wcs_len+1];
	ZeroMemory(pBuf, wcs_len+1);
	if(pBuf!=NULL)
	{
		WideCharToMultiByte(CP_ACP,0,strReturn,strReturn.GetLength(),pBuf,wcs_len,NULL,NULL);
	}
	//MultiByteToWideChar
	WriteFile(hFile,pBuf,(DWORD)strlen(pBuf),&reSize,NULL);
	if (pBuf!=NULL)
	{
		delete [] pBuf;
		pBuf = NULL;
	}
	if (pPtr!=NULL)
	{
		delete [] pPtr;
		pPtr = NULL;
	}
	
	CloseHandle(hFile);
	index = _taccess(strFilePath,0);
	if (index!=-1)
	{
		ShellExecute(NULL,NULL,strFilePath,NULL,NULL,SW_SHOW);
		Sleep(300);
		//删除文件
		DeleteFile(strFilePath);
	}
	
	return;
}

CString GetPackageName(CString strFilePath)
{
	HANDLE hFile;//文件句柄
	int fileSize = 0;
	int wcs_len = 0;
	DWORD reSize = 0;
	CString str;
	CHAR *pBuf = NULL;
	TCHAR *pPtr = NULL;
	CString strReturn;
	CString strContent;
	CString strTemp;
	int index = -1;

	hFile=CreateFile(
		strFilePath,					
		GENERIC_READ,			
		FILE_SHARE_READ,						
		NULL,					
		OPEN_EXISTING,			
		NULL,	
		NULL);

	if(hFile==INVALID_HANDLE_VALUE)
	{
		return _T("");
	}

	fileSize = GetFileSize(hFile,NULL);
	pBuf = new CHAR[fileSize+1];

	ReadFile(hFile,pBuf,fileSize,&reSize,NULL);
	pBuf[fileSize]='\0';


	wcs_len=MultiByteToWideChar(CP_UTF8,0,pBuf,-1,NULL,NULL);
	pPtr=new TCHAR[wcs_len + 1];  //分配内存空间
	ZeroMemory(pPtr, (wcs_len+1) * sizeof(TCHAR));
	if(pPtr!=NULL)
	{ 
		//将char字串转换为wchar_t字串
		MultiByteToWideChar(CP_UTF8,0,pBuf,(int)strlen(pBuf),pPtr,wcs_len);
	}

	if (pBuf!=NULL)
	{
		delete [] pBuf;
		pBuf = NULL;
	}

	//应用软件名称：
	strContent.Format(_T("%s"),pPtr);
	index = strContent.Find(_T("package: name="));
	strContent.Delete(0,index+15);
	index = strContent.Find(_T('\''));
	strContent = strContent.Left(index);
	strReturn = strContent;

	if (pPtr!=NULL)
	{
		delete [] pPtr;
		pPtr = NULL;
	}
	
	CloseHandle(hFile);
	return strReturn;
}
unsigned int __stdcall GetCopyPath(PVOID pm)
{
	ThreadData* plugData = (ThreadData*)pm;
	CString filePath = plugData->strFilePath;
	if (!filePath.IsEmpty())
	{
		if (OpenClipboard(NULL))   //如果能打开剪贴板
		{
			::EmptyClipboard();  //清空剪贴板，使该窗口成为剪贴板的拥有者
			HGLOBAL hClip;
			hClip = ::GlobalAlloc(GMEM_MOVEABLE, (filePath.GetLength() * 2) + 2); 
			TCHAR *pBuf;
			pBuf = (TCHAR *)::GlobalLock(hClip);//锁定剪贴板
			lstrcpy(pBuf, filePath);//把CString转换
			::GlobalUnlock(hClip);//解除锁定剪贴板
			::SetClipboardData(CF_UNICODETEXT, hClip);//把文本数据发送到剪贴板  CF_UNICODETEXT为Unicode编码
			::CloseClipboard();//关闭剪贴板
			ShowTips(_T("网易易盾加固提醒您"), _T("复制路径成功！"));
			return 1;

		}
	}
	else
	{
		ShowTips(_T("网易易盾加固提醒您"), _T("复制路径出现了一点小意外"));
		return 0;
	}
}

void GetSignKey(CString strFilePath)
{
	HANDLE hFile;//文件句柄
	int fileSize = 0;
	int wcs_len = 0;
	DWORD reSize = 0;
	CHAR *pBuf = NULL;
	TCHAR *pPtr = NULL;
	CString strReturn;
	int index = -1;

	hFile=CreateFile(
		strFilePath,					
		GENERIC_READ|GENERIC_WRITE,			
		FILE_SHARE_READ,						
		NULL,					
		OPEN_EXISTING,			
		NULL,	
		NULL);

	if(hFile==INVALID_HANDLE_VALUE)
	{
		return;
	}

	fileSize = GetFileSize(hFile,NULL);
	pBuf = new CHAR[fileSize+1];

	ReadFile(hFile,pBuf,fileSize,&reSize,NULL);
	pBuf[fileSize]='\0';


	wcs_len=MultiByteToWideChar(CP_ACP,0,pBuf,-1,NULL,NULL);
	pPtr=new TCHAR[wcs_len + 1];  //分配内存空间
	ZeroMemory(pPtr, (wcs_len+1) * sizeof(TCHAR));
	if(pPtr!=NULL)
	{ 
		//将char字串转换为wchar_t字串
		MultiByteToWideChar(CP_ACP,0,pBuf,(int)strlen(pBuf),pPtr,wcs_len);
	}

	if (pBuf!=NULL)
	{
		delete [] pBuf;
		pBuf = NULL;
	}

	//格式化：
	//szReturn.Format(_T("%d.\r\n"),indexOf);
	strReturn.Format(_T("\r\n%s"),pPtr);
	index = strReturn.Find(_T("所有者:"));
	strReturn.Insert(index,_T("  "));
	index = strReturn.Find(_T("发布者:"));
	strReturn.Insert(index,_T("\r\n  "));
	index = strReturn.Find(_T("序列号:"));
	strReturn.Insert(index,_T("\r\n  "));
	index = strReturn.Find(_T("有效期开始日期:"));
	strReturn.Insert(index,_T("\r\n  "));
	index = strReturn.Find(_T("截止日期:"));
	strReturn.Insert(index,_T("\r\n  "));
	index = strReturn.Find(_T("证书指纹:"));
	strReturn.Insert(index,_T("\r\n  "));
	index = strReturn.Find(_T("MD5:"));
	strReturn.Insert(index,_T("\r\n      "));
	index = strReturn.Find(_T("SHA1:"));
	strReturn.Insert(index,_T("\r\n      "));
	index = strReturn.Find(_T("SHA256:"));
	strReturn.Insert(index,_T("\r\n      "));
	index = strReturn.Find(_T("签名算法名称:"));
	strReturn.Insert(index,_T("\r\n  "));
	index = strReturn.Find(_T("版本:"));
	strReturn.Insert(index,_T("\r\n  "));

	SetFilePointer (hFile, 0, NULL, FILE_BEGIN);
	SetEndOfFile(hFile);

	wcs_len = WideCharToMultiByte(CP_ACP,0,strReturn,-1,NULL,0,NULL,NULL);
	pBuf = new CHAR[wcs_len+1];
	ZeroMemory(pBuf, wcs_len+1);
	if(pBuf!=NULL)
	{
		WideCharToMultiByte(CP_ACP,0,strReturn,strReturn.GetLength(),pBuf,wcs_len,NULL,NULL);
	}
	//MultiByteToWideChar
	WriteFile(hFile,pBuf,(DWORD)strlen(pBuf),&reSize,NULL);

	if (pBuf!=NULL)
	{
		delete [] pBuf;
		pBuf = NULL;
	}
	
	if (pPtr!=NULL)
	{
		delete [] pPtr;
		pPtr = NULL;
	}

	CloseHandle(hFile);
	
	index = _taccess(strFilePath,0);
	if (index!=-1)
	{
		ShellExecute(NULL,NULL,strFilePath,NULL,NULL,SW_SHOW);
	}
	return;
}

CStringW CStrA2CStrW(const CStringA &cstrSrcA)  
{  
	int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, NULL, 0);  
	wchar_t *wstr = new wchar_t[len];  
	memset(wstr, 0, len * sizeof(wchar_t));  
	MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, wstr, len);  
	CStringW cstrDestW = wstr;  
	delete wstr;  
	return cstrDestW;  
} 

void PathWithSlash(CStringA&strPath)
{
	if (strPath.GetLength() > 1) {
		strPath.Replace('/', '\\');
		if (strPath.Right(1) != "\\") {
			strPath += "\\";
		}
	}
}

void PathWithSlashW(CStringW&strPath)
{
	if (strPath.GetLength() > 1) {
		strPath.Replace('/', '\\');
		if (strPath.Right(1) != "\\") {
			strPath += "\\";
		}
	}
}

int GetSubFolder(OUT vector<CStringA> &vtFolder, IN LPCSTR lpszRoot, IN LPCSTR lpszFilter)
{
	int         iRetVal = 0;
	CStringA		strRoot;

	WIN32_FIND_DATAA    Win32FindData = { 0 };
	HANDLE              hFindHandle = NULL;

	if (lpszRoot == NULL)
	{
		iRetVal = -1;
		goto fun_ret;
	}

	strRoot = lpszRoot;

	if (strRoot.IsEmpty())
	{
		iRetVal = -2;
		goto fun_ret;
	}

	PathWithSlash(strRoot);

	hFindHandle = FindFirstFileA(strRoot + "*", &Win32FindData);
	if (hFindHandle == INVALID_HANDLE_VALUE)
	{
		iRetVal = -3;
		goto fun_ret;
	}

	if (!(strlen(Win32FindData.cFileName) == 1 && strncmp(Win32FindData.cFileName, ".", 1) == 0)
		&& !(strlen(Win32FindData.cFileName) == 2 && strncmp(Win32FindData.cFileName, "..", 2) == 0))
	{
		CStringA strFolderName = Win32FindData.cFileName;
		if ((Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (strFolderName.MakeLower().Compare(lpszFilter) != 0))
		{
			vtFolder.push_back(Win32FindData.cFileName);
		}
	}
	while (FindNextFileA(hFindHandle, &Win32FindData))
	{
		if (!(strlen(Win32FindData.cFileName) == 1 && strncmp(Win32FindData.cFileName, ".", 1) == 0)
			&& !(strlen(Win32FindData.cFileName) == 2 && strncmp(Win32FindData.cFileName, "..", 2) == 0))
		{
			CStringA strFolderName = Win32FindData.cFileName;
			if ((Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (strFolderName.MakeLower().Compare(lpszFilter) != 0))
			{
				vtFolder.push_back(Win32FindData.cFileName);
			}
		}
	}

	if (hFindHandle != INVALID_HANDLE_VALUE)
		FindClose(hFindHandle);

fun_ret:
	return iRetVal;
}

//检查python是否配置和python版本
int CheckPython()
{
	int isHave;
	CString strCmdLine("cmd /c python -V");
	CStringA strOutPut = RunCmdAndOutput(strCmdLine);
	int index = strOutPut.Find(" ");
	CStringA version = strOutPut.Mid(index+1, 1);
	if (version == "2")
	{
		isHave = 2;
	}
	else if(version == "3")
	{
		isHave = 3;
	}
	else
	{
		isHave = -1;
	}
	return isHave;

}

BOOL GetKeytoolPath(CStringA &strJarSingerPath)
{
	BOOL bRet = FALSE;
	CString strCmdLine;
	CStringA strFindFlag = "lib\\rt.jar]";
	CStringA strJavaFlag = "\\java\\";
	int pos = -1;
	vector<CStringA> vtFolder;
	CStringA strFilter = "***";

	strCmdLine = _T("cmd /c java -verbose");

	CStringA strOutPut = RunCmdAndOutput(strCmdLine);
	strOutPut.Replace('/', '\\');

	pos = strOutPut.Find("lib\\rt.jar]");
	if (pos != -1)
	{
		strOutPut = strOutPut.Left(pos + strFindFlag.GetLength() - 1);
		pos = strOutPut.ReverseFind('[');
		if (pos != -1)
		{
			strOutPut.Delete(0, pos + 1);
		}

		CStringA strBeforPath = strOutPut;
		pos = strBeforPath.Find(":\\");
		if (pos != -1)
		{
			strBeforPath = strBeforPath.Left(pos);
		}

		pos = strBeforPath.ReverseFind(' ');
		if (pos != -1)
		{
			strOutPut.Delete(0, pos + 1);
		}

		//CUtils::UnitTest("out.txt", strOutPut + "\r\n");

		strBeforPath = strOutPut;
		strBeforPath.MakeLower();
		pos = strBeforPath.Find("\\java\\");
		if (pos != -1)
		{
			strOutPut = strOutPut.Left(pos + strJavaFlag.GetLength() - 1);
		}
		//遍历目录

		if (_access(strOutPut, 0) == 0 && !strFilter.IsEmpty())
		{
			GetSubFolder(vtFolder, strOutPut, strFilter);
			PathWithSlash(strOutPut);
			for (vector<CStringA>::iterator it = vtFolder.begin(); it != vtFolder.end(); it++)
			{
				strJarSingerPath = strOutPut + *it + "\\bin\\keytool.exe";
		
				if (_access(strJarSingerPath, 0) == 0)
				{
					bRet = TRUE;
					break;
				}
				else
				{
					strJarSingerPath = "";
				}
			}
		}
		else
		{
			strJarSingerPath = "";
		}
	}
	else
	{
		//CUtils::UnitTest("out.txt", "ggg没找到\r\n");
		strJarSingerPath = "";
	}

	return bRet;
}

void GetErrorMsg(LPCTSTR decFile, CString & outPut)
{
	LPVOID lpMsgBuf = NULL;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	outPut.Format(_T("%s%s"), decFile, lpMsgBuf);

	if (lpMsgBuf)
		LocalFree(lpMsgBuf);
}

BOOL TraverseFolderAndMVApk(LPCTSTR lpSrcPath, LPCTSTR lpszDesPath, CString &strOutput)
{
	BOOL ret = FALSE;
	HANDLE	hFindHandle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA Win32FindData = { 0 };
	CString		strOldFilePath;
	CString     strNewFilePath;

	strOldFilePath = lpSrcPath;
	strNewFilePath = lpszDesPath;

	PathWithSlashW(strOldFilePath);
	PathWithSlashW(strNewFilePath);

	if (_taccess(strOldFilePath, 0) != 0)
	{
		ret = FALSE;
		goto fun_ret;
	}

	hFindHandle = FindFirstFile(strOldFilePath + _T("*"), &Win32FindData);
	if (hFindHandle == INVALID_HANDLE_VALUE)
	{
		ret = FALSE;
		goto fun_ret;
	}

	if (!(_tcslen(Win32FindData.cFileName) == 1 && _tcsncmp(Win32FindData.cFileName, _T("."), 1) == 0)
		&& !(_tcslen(Win32FindData.cFileName) == 2 && _tcsncmp(Win32FindData.cFileName, _T(".."), 2) == 0))
	{
		if (!(Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CString strFileName;
			strFileName.Format(_T("%s"), Win32FindData.cFileName);
			if (strFileName.MakeLower().Find(_T(".apk")) != -1)
			{
				if (!MoveFileEx(strOldFilePath + Win32FindData.cFileName, strNewFilePath + Win32FindData.cFileName, MOVEFILE_REPLACE_EXISTING))
				{
					ret = FALSE;
					GetErrorMsg(strNewFilePath + strFileName, strOutput);
					goto fun_ret;
				}
				else 
				{
					ret = TRUE;
					strOutput = strNewFilePath + strFileName;
					goto fun_ret;
				}
			}
		}
		else if (Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		}
	}

	while (FindNextFile(hFindHandle, &Win32FindData))
	{
		if (!(_tcslen(Win32FindData.cFileName) == 1 && _tcsncmp(Win32FindData.cFileName, _T("."), 1) == 0)
			&& !(_tcslen(Win32FindData.cFileName) == 2 && _tcsncmp(Win32FindData.cFileName, _T(".."), 2) == 0))
		{
			if (!(Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				CString strFileName;
				strFileName.Format(_T("%s"), Win32FindData.cFileName);
				if (strFileName.MakeLower().Find(_T(".apk")) != -1)
				{
					if (!MoveFileEx(strOldFilePath + Win32FindData.cFileName, strNewFilePath + Win32FindData.cFileName, MOVEFILE_REPLACE_EXISTING))
					{
						ret = FALSE;
						GetErrorMsg(strNewFilePath + strFileName, strOutput);
						goto fun_ret;
					}
					else
					{
						ret = TRUE;
						strOutput = strNewFilePath + strFileName;
						break;
					}
				}
			}
			else if (Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
			}
		}
	}


fun_ret:
	if (hFindHandle != INVALID_HANDLE_VALUE)
		FindClose(hFindHandle);

	return ret;
}


//get apk signinfo
unsigned int __stdcall ThreadGetSigninfo(PVOID pM)
{
	CString fileType;
	CString strUnZipPath;
	CString strCmdLine;
	CString strJavaExist;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	fileType = strFileName;
	strUnZipPath = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	CStringA strKeytoolPath;
	CStringA strOutput;

	/*strCmdLine = _T("cmd.exe /c keytool.exe");
	strJavaHome = RunCmdAndOutput(strCmdLine);*/

	BOOL bRet = GetKeytoolPath(strKeytoolPath);
	if (!bRet)
		strKeytoolPath = "";
	//MessageBox(NULL, CA2W(strKeytoolPath), NULL, NULL);
	if(!fileType.CompareNoCase(_T(".apk")))
	{
		strUnZipPath.Replace(_T(".apk"),_T("_signinfo.txt"));
		
		if (!strKeytoolPath.IsEmpty())
		{
			strKeytoolPath.Replace('/','\\');

			CString strKeytool;
			strKeytool = CStrA2CStrW(strKeytoolPath);
	
			//strCmdLine = _T("\"")+strKeytool + _T("bin\\keytool.exe\"  -printcert -jarfile \"");
			strCmdLine = _T("\"") + strKeytool + _T("\"  -printcert -jarfile \"");
			strCmdLine += strFileName;	
			strCmdLine += (_T("\""));

			strOutput = RunCmdAndOutput(strCmdLine);

			WriteLogFile(strUnZipPath,strOutput);
			
			GetSignKey(strUnZipPath);
			strJavaExist = _T(" Ok!");
		}
		else
		{
			strJavaExist = _T(" 失败！请添加JAVA_HOME到系统环境变量!");
		}
		
		strCmdLine.Empty();
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;
		
		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	strFileName += strJavaExist;
	
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName = _T("");
	if (plugData)
		delete plugData;

	return 0;
}

//get apk signinfo
unsigned int __stdcall ThreadGetWrapperInfo(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	CString pyFile;
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strRet=_T("");
	CString strLogFile;

	fileType = strFileName;
	strLogFile = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		//MessageBox(NULL, _T("python2"), NULL, 0);
		pyFile = _T("ApkDetect.py");
	}
	else
	{
		pyFile = _T("ApkDetect3.py");
	}

	if(!fileType.CompareNoCase(_T(".apk")))
	{	
		strLogFile.Replace(_T(".apk"), _T("_wrapper.txt"));
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" ");
		strCmdLine += szModulePath;
		strCmdLine += _T(" ");
		strCmdLine += _T("0");

		CStringA strRetA = RunCmdAndOutput(strCmdLine);
		if (!strRetA.IsEmpty())
		{
				strRet = CStrA2CStrW(strRetA);

				WriteLogFile(strLogFile, "加壳信息: " + strRetA);
				if (_taccess_s(strLogFile, 0) == 0)
				{
					ShellExecute(NULL, NULL, strLogFile, NULL, NULL, SW_SHOW);
					Sleep(300);
					//删除文件
					DeleteFile(strLogFile);
				}
			
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;
		
		return 0;
	}

	ShowTips(_T("网易易盾加固提醒您"), strRet);
	if (plugData)
		delete plugData;

	return 0;
}
unsigned int __stdcall ThreadDeepDetect(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	CString pyFile;
	int index = 0;
	if (!pM)
		return -1;
	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strRet = _T("");
	CString strLogFile;

	fileType = strFileName;
	strLogFile = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0, index);
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		//MessageBox(NULL, _T("python2"), NULL, 0);
		pyFile = _T("deepDetect.py");
	}
	else
	{
		pyFile = _T("deepDetect3.py");
	}

	if (!fileType.CompareNoCase(_T(".apk")))
	{
		strLogFile.Replace(_T(".apk"), _T("_info.txt"));
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" ");
		strCmdLine += szModulePath;

		//MessageBox(NULL,strCmdLine,NULL,0);

		CStringA strRetA = RunCmdAndOutput(strCmdLine);
		//CStringA strApkPathA;
		//strApkPathA = strFileName;
		if (!strRetA.IsEmpty())
		{
			strRet = CStrA2CStrW(strRetA);

			WriteLogFile(strLogFile,strRetA);
			if (_taccess_s(strLogFile, 0) == 0)
			{
				ShellExecute(NULL, NULL, strLogFile, NULL, NULL, SW_SHOW);
				Sleep(300);
				//删除文件
				DeleteFile(strLogFile);
			}

		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;
		return 0;
	}

	ShowTips(_T("网易易盾加固提醒您"), strRet);
	if (plugData)
		delete plugData;

	return 0;
}
unsigned int __stdcall ThreadApkAlign(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	CString pyFile;
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strRet = _T("");

	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0, index);

	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("ToolsCall.py");
	}
	else
	{
		pyFile = _T("ToolsCall3.py");
	}
	if (!fileType.CompareNoCase(_T(".apk")))
	{
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" ");
		strCmdLine += szModulePath;
		strCmdLine += _T(" ");
		strCmdLine += _T("align");

		//MessageBox(NULL,strCmdLine,NULL,0);
		CStringA strRetA = RunCmdAndOutput(strCmdLine);
		if (!strRetA.IsEmpty())
		{
				strRet = CStrA2CStrW(strRetA);
				strRet = strRet.Trim();
			//strRet = CStrA2CStrW(strRetA);
			//strRet = strRet.Trim();
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	ShowTips(_T("网易易盾加固提醒您"), strRet);
	if (plugData)
		delete plugData;

	return 0;
}

unsigned int __stdcall ThreadGetExtraIcon(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strRet=_T("");
	//CString strLogFile;
	CString pyFile;
	fileType = strFileName;
	//strLogFile = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("ApkDetect.py");
	}
	else
	{
		pyFile = _T("ApkDetect3.py");
	}
	if(!fileType.CompareNoCase(_T(".apk")))
	{	
		//strLogFile.Replace(_T(".apk"), _T("_wrapper.log"));
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" ");
		strCmdLine += szModulePath;
		strCmdLine += _T(" ");
		strCmdLine += _T("1");

		//MessageBox(NULL,strCmdLine,NULL,0);

		CStringA strRetA = RunCmdAndOutput(strCmdLine);
		strRetA = strRetA.Trim();
		
		if (!strRetA.IsEmpty())
		{
			//strRet = CStrA2CStrW(strRetA);
				strRet = CStrA2CStrW(strRetA);
				strRet = strRet.Trim();
		}
		else 
		{
			strRet = _T("提取图标出错");
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;
		
		return 0;
	}

	ShowTips(_T("网易易盾加固提醒您"), _T("处理结果：") + strRet);
	if (plugData)
		delete plugData;

	return 0;
}
unsigned int __stdcall ThreadGetExtraLib(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strRet = _T("");
	//CString strLogFile;
	CString pyFile;
	fileType = strFileName;
	//strLogFile = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0, index);
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("ApkDetect.py");
	}
	else
	{
		pyFile = _T("ApkDetect3.py");
	}
	if (!fileType.CompareNoCase(_T(".apk")))
	{
		//strLogFile.Replace(_T(".apk"), _T("_wrapper.log"));
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" ");
		strCmdLine += szModulePath;
		strCmdLine += _T(" ");
		strCmdLine += _T("2");

		//MessageBox(NULL,strCmdLine,NULL,0);

		CStringA strRetA = RunCmdAndOutput(strCmdLine);
		strRetA = strRetA.Trim();

		if (!strRetA.IsEmpty())
		{
			//strRet = CStrA2CStrW(strRetA);
			strRet = CStrA2CStrW(strRetA);
			strRet = strRet.Trim();
		}
		else
		{
			strRet = _T("提取库文件出错");
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	ShowTips(_T("网易易盾加固提醒您"), _T("处理结果：") + strRet);
	if (plugData)
		delete plugData;

	return 0;
}

unsigned int __stdcall ThreadIsRegular(PVOID pM)
{
	CString fileType;

	CString strCmdLine;
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strRet = _T("");
	CString pyFile;
	CString strLogFile = strFileName;
	strLogFile.Replace(_T(".apk"), _T("_regular.txt"));
	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0, index);
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("ApkDetect.py");
	}
	else
	{
		pyFile = _T("ApkDetect3.py");
	}
	if (!fileType.CompareNoCase(_T(".apk")))
	{
		//strLogFile.Replace(_T(".apk"), _T("_wrapper.log"));
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" ");
		strCmdLine += szModulePath;
		strCmdLine += _T(" ");
		strCmdLine += _T("3");

		//MessageBox(NULL,strCmdLine,NULL,0);

		CStringA strRetA = RunCmdAndOutput(strCmdLine);
		strRetA = strRetA.Trim();
		CString aa(strRetA);
		if (!strRetA.IsEmpty())
		{
			//strRet = CStrA2CStrW(strRetA);
			strRet = _T("lib规范性检查成功");
			WriteLogFile(strLogFile, strRetA);
			if (_taccess_s(strLogFile, 0) == 0)
			{
				ShellExecute(NULL, NULL, strLogFile, NULL, NULL, SW_SHOW);
				Sleep(300);
				//删除文件
				DeleteFile(strLogFile);
			}
		}
		else
		{
			strRet = _T("lib规范性检查失败");
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	ShowTips(_T("网易易盾加固提醒您"), _T("处理结果：") + strRet);
	if (plugData)
		delete plugData;

	return 0;
}


unsigned int __stdcall ThreadCustomPlugFunc(PVOID pM)
{
	CString strFileName;
	CString strPlugIndex;
	CString strCmdLine;
	CString pyFile;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;
	CString strRet = _T("");
	
	strFileName = plugData->strFilePath;
	strPlugIndex = plugData->strPlugIndex;
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("CustomPlug.py");
	}
	else
	{
		pyFile = _T("CustomPlug3.py");
	}
	strCmdLine = _T("cmd /c python ");
	strCmdLine += _T("\"");
	strCmdLine += szModulePath;
	strCmdLine += _T("python\\");
	strCmdLine += pyFile + _T("\" \"");
	strCmdLine += strFileName;
	strCmdLine += _T("\" ");
	strCmdLine += szModulePath;
	strCmdLine += _T(" ");
	strCmdLine += strPlugIndex;
	//MessageBox(NULL, szModulePath, NULL,0);
	CStringA strRetA = RunCmdAndOutput(strCmdLine);
	strRetA = strRetA.Trim();

	if (!strRetA.IsEmpty())
	{
		//strRet = CStrA2CStrW(strRetA);
		strRet = CStrA2CStrW(strRetA);
		strRet = strRet.Trim();
	}
	else
	{
		strRet = _T("自定义插件函数运行出错");
	}
	

	ShowTips(_T("网易易盾加固提醒您"), _T("处理结果：") + strRet);
	if (plugData)
		delete plugData;
		
	return 0;
}

unsigned int __stdcall ThreadScreenCap(PVOID pM)
{
	CString strCmdLine, strTime;
	CString strRet = _T("");
	CString strScreenCapPath;
	CString strConFileName;
	CString pyFile;
	//CString strAdbPath = _T("adb");
	CString strAdbPath = _T("");
	strAdbPath += szModulePath;
	strAdbPath += _T("adb\\adb.exe");

	strConFileName = szModulePath;
	strConFileName += _T("config\\config.ini");

	if (!pM)
		return -1;
	ThreadData* plugData = (ThreadData*)pM;
	CString strFileName = plugData->strFilePath;
	int index= strFileName.ReverseFind('\\');
	CString strFilePath = strFileName.Left(index);
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("adb.py");
	}
	else
	{
		pyFile = _T("adb3.py");
	}
	int scales = GetPrivateProfileInt(_T("ScaleSize"), _T("Scale"), 50, strConFileName);
		if (scales <= 0)
			scales = 25;

		float size = (float)(scales / 100.0);

		//MessageBox(NULL, strAdbPath, NULL, 0);
		strScreenCapPath = strFilePath;
		strScreenCapPath+=_T("\\screenshot.png");
		//MessageBox(NULL, strScreenCapPath, NULL, 0);
		strCmdLine = _T("cmd /c python ");
		strCmdLine += _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strAdbPath;
		strCmdLine += _T("\" ");
		strCmdLine += strFilePath;
		strCmdLine += _T(" screenCap");
		//MessageBox(NULL, strCmdLine, NULL, 0);
		CStringA strRetA = RunCmdAndOutput(strCmdLine);

		if (!strRetA.IsEmpty())
		{
			if (strRetA.Find("设备列表为空，请连接设备") == -1)
			{
				strRetA = strRetA.Trim();
				strRet = CStrA2CStrW(strRetA);

				if (_taccess_s(strScreenCapPath, 0) == 0)
				{
					//MessageBox(NULL, _T("sucess"), NULL, 0);
					CString strPngFile = ScalePng(strScreenCapPath, size, size);
					if (_taccess(strPngFile, 0) == 0)
					{
						CString strOutput;
						if (SetPngToClipboard(strPngFile, strOutput) == FALSE)
							strRet = strOutput;
						else
							strRet = _T("手机截图成功");
					}
					else
					{
						strRet = _T("图像缩放失败");
					}

					//DeleteFile(strScreenCapPath);
				}
			}
			else
			{
				strRet = _T("设备列表为空，请连接设备");
			}
		}
		else
		{
			strRet = _T("手机截图失败");
		}
	ShowTips(_T("网易易盾加固提醒您"), strRet);
	
	return 0;
}


unsigned int __stdcall ThreadGetPhoneInfo(PVOID pM)
{
	CString strCmdLine;
	CString strRet = _T("");
	//CString strAdbPath = _T("adb");
	CString strAdbPath = _T("");
	strAdbPath += szModulePath;
	strAdbPath += _T("adb\\adb.exe");
	CString strLogFile;
	CString pyFile;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	strLogFile = strFileName;

	/*BOOL bRet = CheckAdb();
	if (!bRet)
	{
		strRet = _T("请在环境变量中配置adb路径!");
		goto _SHOW_RESULT;
	}*/
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strRet = _T("请安装python，并配置pyhton到环境变量");
		ShowTips(_T("网易易盾加固提醒您"), strRet);
		if (plugData)
			delete plugData;
		return 0;
	}
	else if (bRet == 2)
	{
		pyFile = _T("adb.py");
	}
	else
	{
		pyFile = _T("adb3.py");
	}
	strCmdLine = _T("");
	strLogFile += _T("_phoneInfo.log");
	strCmdLine = _T("cmd /c python ");
	strCmdLine += _T("\"");
	strCmdLine += szModulePath;
	strCmdLine += _T("python\\");
	strCmdLine += pyFile + _T("\" \"");
	strCmdLine += strAdbPath;
	strCmdLine += _T("\" ");
	strCmdLine += _T("getPhoneInfo");
	strCmdLine += _T(" getinfo");

	CStringA strRetA = RunCmdAndOutput(strCmdLine);
		//MessageBoxA(NULL, strRetA, NULL, 0);

	if (!strRetA.IsEmpty())
	{
		if (strRetA.Find("设备列表为空，请连接设备") == -1)
		{
			strRetA = strRetA.Trim();
			strRet = _T("获取手机信息成功");
				
			WriteLogFile(strLogFile, "" + strRetA);
			if (_taccess_s(strLogFile, 0) == 0)
			{
				ShellExecute(NULL, NULL, strLogFile, NULL, NULL, SW_SHOW);
				Sleep(300);
					//删除文件
				DeleteFile(strLogFile);
			}
		}
		else
		{
			strRet = _T("设备列表为空，请连接设备");
		}
	}
	else
	{
		strRet = _T("获取手机信息失败");
	}
	ShowTips(_T("网易易盾加固提醒您"), strRet);
	if (plugData)
		delete plugData;
	
	return 0;
}


//apk sign
unsigned int __stdcall ThreadApkSign(PVOID pM)
{
	CString fileType;
	CString szCmdLine;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;
	CString strLogFile;

	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		szCmdLine = _T("\"");
		szCmdLine += szModulePath;
		szCmdLine += (_T("sign\\sign.bat\" \""));
		szCmdLine += strFileName;
		szCmdLine += _T("\"");

		//RunCmdFile(szCmdLine);
		strLogFile = strFileName;
		strLogFile.Replace(_T(".apk"),_T("_sign.txt"));
		RunCmdAndWriteFile(szCmdLine, strLogFile);
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	strFileName += _T("处理完成");
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	
	if (plugData)
		delete plugData;

	return 0;
}


//apk uninstall
unsigned int __stdcall ThreadApkUninstall(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	CString strJarName;
	CString strReturn;
	CString pyFile;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	CString strLog;
	TCHAR * pPtr = NULL;

	//CString strConFileName;
	//CString strAdbPath = _T("adb");
	CString strAdbPath = _T("");
	strAdbPath += szModulePath;
	strAdbPath += _T("adb\\adb.exe");
	//BOOL bRet = CheckAdb();
	//if (!bRet)
	///{
	//	strLog = _T("请在环境变量中配置adb路径!");
	//	goto _SHOW_RESULT;
	//}
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strLog = _T("请安装python，并配置pyhton到环境变量");
		goto _SHOW_RESULT;
	}
	else if (bRet == 2)
	{
		pyFile = _T("adb.py");
	}
	else
	{
		pyFile = _T("adb3.py");
	}

	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		strJarName = strFileName;
		strJarName.Replace(_T(".apk"),_T(".txt"));
		strCmdLine = _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += (_T("aapts.bat\" dump badging \""));
		strCmdLine += strFileName;
		strCmdLine += (_T("\" > \"")+strJarName);
		strCmdLine += _T("\"");

		RunCmdFile(strCmdLine);
		//
		strReturn = GetPackageName(strJarName);
		index = _taccess(strJarName,0);
		if (index == 0)
		{
			DeleteFile(strJarName);
		}
		
		strJarName.Empty();

		////
		strCmdLine = _T("cmd /c python \"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strAdbPath;
		strCmdLine += _T("\" ");
		strCmdLine += strReturn;
		strCmdLine += _T(" uninstall");

		CStringA strOutput;
		strOutput = RunCmdAndOutput(strCmdLine);
		strOutput = strOutput.Trim();

		if (strOutput.Find("不是内部或外部命令") == -1)
		{
			index = MultiByteToWideChar(CP_ACP, 0, strOutput.GetBuffer(), -1, NULL, NULL);
			pPtr = new TCHAR[index + 1];  //分配内存空间
			ZeroMemory(pPtr, (index + 1) * sizeof(TCHAR));
			if (pPtr != NULL)
			{
				//将char字串转换为wchar_t字串
				MultiByteToWideChar(CP_ACP, 0, strOutput.GetBuffer(), (int)strlen(strOutput.GetBuffer()), pPtr, index);
			}

			strLog.Format(_T("%s"), pPtr);
			strOutput.ReleaseBuffer();
			if (pPtr != NULL)
			{
				delete[] pPtr;
				pPtr = NULL;
			}

			strCmdLine.Empty();
		}
		else
		{
			strLog = _T("请安装python，并配置pyhton到环境变量");
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

_SHOW_RESULT:
	ShowTips(_T("网易易盾加固提醒您"),strLog);
	strFileName.Empty();
	strLog.Empty();

	if (plugData)
		delete plugData;

	return 0;
}


//apk install 卸载安装
unsigned int __stdcall ThreadApkInstall(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	CString strJarName;
	CString strReturn;
	CString strNameTemp;
	CString strTime;
	CString pyFile;
	int index = 0;
	int indexName = -1;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	CString strLog;
	TCHAR * pPtr = NULL;

	CString strAdbPath = _T("");
	strAdbPath += szModulePath;
	strAdbPath += _T("adb\\adb.exe");

	int bRet = CheckPython();
	if (bRet== -1)
	{
		strLog = _T("请安装python，并配置pyhton到环境变量");
		goto _SHOW_RESULT;
	}
	else if (bRet == 2)
	{
		pyFile = _T("adb.py");
	}
	else
	{
		pyFile = _T("adb3.py");
	}
	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		//获取包名
		strJarName = strFileName;
		strJarName.Replace(_T(".apk"),_T(".txt"));
		strCmdLine = _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += (_T("aapts.bat\" dump badging \""));
		strCmdLine += strFileName;
		strCmdLine += _T("\"");
		strCmdLine += (_T(" > \"")+strJarName);
		strCmdLine += _T("\"");
		RunCmdFile(strCmdLine);
		//
		strReturn = GetPackageName(strJarName);
		index = _taccess(strJarName,0);
		if (index == 0)
		{
			DeleteFile(strJarName);
		}

		strJarName.Empty();
		//卸载
		strCmdLine = _T("cmd /c python \"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine+=pyFile+_T("\" \"");
		strCmdLine += strAdbPath;
		strCmdLine += _T("\" ");
		strCmdLine += strReturn;
		strCmdLine += _T(" uninstall");
		//MessageBox(NULL, strCmdLine, NULL, 0);
		//RunCmdFile(strCmdLine);
		CStringA strOutputA;
		strOutputA = RunCmdAndOutput(strCmdLine);
		strOutputA = strOutputA.Trim();

		///正常安装
		strCmdLine = _T("cmd /c python \"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strAdbPath;
		strCmdLine += _T("\" \"");
		
		strNameTemp = strFileName;
		indexName = strNameTemp.ReverseFind('\\');
		if (indexName != -1)
		{
			strNameTemp = strNameTemp.Left(indexName+1);
			
			//中文文件名处理
			SYSTEMTIME st;
			GetLocalTime(&st);
			strTime.Format(_T("%02d-%02d-%02d-%02d.apk"), st.wDay, st.wHour, st.wMinute, st.wSecond);
			
			strNameTemp += strTime;
			if (CopyFileEx(strFileName, strNameTemp, NULL, NULL, FALSE, COPY_FILE_FAIL_IF_EXISTS)==TRUE)
			{
				strCmdLine += strNameTemp;
				strCmdLine += _T("\"");
			}
			else
			{
				strNameTemp.Empty();
				strCmdLine += strFileName;
				strCmdLine += _T("\"");
			}
		}else
		{
			strNameTemp.Empty();
			strCmdLine += strFileName;
			strCmdLine += _T("\"");
		}

		CStringA strOutput;
		strCmdLine += _T(" install");
		strOutput = RunCmdAndOutput(strCmdLine);
		
		index = MultiByteToWideChar(CP_ACP,0,strOutput.GetBuffer(),-1,NULL,NULL);
		pPtr=new TCHAR[index + 1];  //分配内存空间
		ZeroMemory(pPtr, (index+1) * sizeof(TCHAR));
		
		if(pPtr!=NULL)
		{ 
			//将char字串转换为wchar_t字串
			MultiByteToWideChar(CP_ACP,0,strOutput.GetBuffer(),(int)strlen(strOutput.GetBuffer()),pPtr,index);
		}

		strLog.Format(_T("%s"),pPtr);
		strOutput.ReleaseBuffer();
		if (pPtr!=NULL)
		{
			delete [] pPtr;
			pPtr = NULL;
		}
		
		strCmdLine.Empty();

		if (!strNameTemp.IsEmpty())
		{
			//DeleteFile(strNameTemp);
			if(_taccess(strNameTemp, 0) == 0)
				DeleteFile(strNameTemp);
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	indexName = strLog.Find(_T("pkg: /d"));
	//MessageBox(NULL,szLog,NULL,MB_OK);
	if(indexName != -1)
	{
		strLog.Delete(0,indexName);
		//MessageBox(NULL,szLog,NULL,MB_OK);
	}

_SHOW_RESULT:
	ShowTips(_T("网易易盾加固提醒您"),strLog);
	strFileName.Empty();
	strLog.Empty();

	if (plugData)
		delete plugData;

	return 0;
}


//apk install 替换安装
unsigned int __stdcall ThreadApkInstallfore(PVOID pM)
{
	CString fileType;
	CString strCmdLine;
	CString strNameTemp;
	CString strTime;
	CString pyFile;
	int index = 0;
	int indexName = -1;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	CString strLog;
	TCHAR * pPtr = NULL;

	//CString strConFileName;
	//CString strAdbPath = _T("adb");
	CString strAdbPath = _T("");
	strAdbPath += szModulePath;
	strAdbPath += _T("adb\\adb.exe");
	//BOOL bRet = CheckAdb();
	int bRet = CheckPython();
	if (bRet == -1)
	{
		strLog = _T("请安装python，并配置pyhton到环境变量");
		goto _SHOW_RESULT;
	}
	else if (bRet == 2)
	{
		pyFile = _T("adb.py");
	}
	else
	{
		pyFile = _T("adb3.py");
	}

	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		strCmdLine = _T("cmd /c python \"");
		strCmdLine += szModulePath;
		strCmdLine += _T("python\\");
		strCmdLine += pyFile + _T("\" \"");
		strCmdLine += strAdbPath;
		strCmdLine += _T("\" \"");
		
		strNameTemp = strFileName;
		indexName = strNameTemp.ReverseFind('\\');
		if (indexName != -1)
		{
			strNameTemp = strNameTemp.Left(indexName+1);
			//中文文件名处理
			SYSTEMTIME st;
			GetLocalTime(&st);
			strTime.Format(_T("%02d-%02d-%02d-%02d.apk"), st.wDay, st.wHour, st.wMinute, st.wSecond);
			strNameTemp += strTime;
			if (CopyFileEx(strFileName, strNameTemp, NULL, NULL, FALSE, COPY_FILE_FAIL_IF_EXISTS)==TRUE)
			{
				strCmdLine += strNameTemp;
				strCmdLine += _T("\"");
			}
			else
			{
				strNameTemp.Empty();
				strCmdLine += strFileName;
				strCmdLine += _T("\"");
			}

		}else
		{
			strNameTemp.Empty();
			strCmdLine += strFileName;
			strCmdLine += _T("\"");
		}

		CStringA strOutput;
		strCmdLine += _T(" install");
		strOutput = RunCmdAndOutput(strCmdLine);
		/*if (strOutput.Find("不是内部或外部命令") != -1)
		{
			strLog = _T("请安装python，并配置pyhton到环境变量");
			if (!strNameTemp.IsEmpty())
			{
				if (_taccess(strNameTemp, 0) == 0)
					DeleteFile(strNameTemp);
			}
			goto _SHOW_RESULT;
		}*/

		index=MultiByteToWideChar(CP_ACP,0,strOutput.GetBuffer(),-1,NULL,NULL);
		pPtr=new TCHAR[index + 1];  //分配内存空间
		ZeroMemory(pPtr, (index+1) * sizeof(TCHAR));
		if(pPtr!=NULL)
		{ 
			//将char字串转换为wchar_t字串
			MultiByteToWideChar(CP_ACP,0,strOutput.GetBuffer(),(int)strlen(strOutput.GetBuffer()),pPtr,index);
		}

		strLog.Format(_T("%s"),pPtr);
		strOutput.ReleaseBuffer();
		if (pPtr!=NULL)
		{
			delete [] pPtr;
			pPtr = NULL;
		}

		strCmdLine.Empty();

		if (!strNameTemp.IsEmpty())
		{
			if (_taccess(strNameTemp, 0) == 0)
				DeleteFile(strNameTemp);
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	indexName = strLog.Find(_T("pkg: /d"));
	
	if(indexName != -1)
	{
		strLog.Delete(0,indexName);
	}

_SHOW_RESULT:
	ShowTips(_T("网易易盾加固提醒您"),strLog);
	strFileName.Empty();
	strLog.Empty();

	if (plugData)
		delete plugData;

	return 0;
}

//回编译
unsigned int __stdcall ThreadComApk(PVOID pM)
{
	CString fileType;
	CString strUnZipPath;
	CString strCmdLine;
	CString strLogName;
	CStringA strOutput;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;
	CString strFileName = plugData->strFilePath;

	if(PathIsDirectory(strFileName))
	{
		strCmdLine = _T("java -jar \"");
		strCmdLine += szModulePath;
		strCmdLine += _T("apktool.jar\" b \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" -o ");
		strCmdLine += strFileName;
		strCmdLine += _T(".apk");
		//MessageBox(NULL, strCmdLine, NULL, 0);
		strLogName = strFileName;
		strLogName += _T("_compile.log");
		
		strOutput = RunCmdAndOutput(strCmdLine);
		
		WriteLogFile(strLogName,strOutput);

		strCmdLine.Empty();
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}
	if (strOutput.Find("error:") == -1 && strOutput.Find("I: Building apk file...") != -1)
	{

		index = _taccess(strLogName, 0);
		if (index != -1)
		{
			DeleteFile(strLogName);
		}
		strFileName = strFileName + _T(".apk OK!");
	} 
	else
	{
		strFileName = _T("Failed，请查看log文件!");
	}
	
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName.Empty();
	if (plugData)
		delete plugData;

	return 0;
}


//反编译
unsigned int __stdcall ThreadDeApk(PVOID pM)
{
	CString fileType;
	CString strUnZipPath;
	CString strCmdLine;
	CString strLogName;
	CStringA strOutput;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	fileType = strFileName;
	strUnZipPath = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		strUnZipPath = strUnZipPath.Left(index);

		strCmdLine = _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += _T("apktool.bat\" d -f \"");
		strCmdLine += strFileName;
		strCmdLine += _T("\" -o \"")+strUnZipPath;
		strCmdLine += _T("\"");

		//RunCmdFile(szCmdLine);
		strLogName = strFileName;
		strLogName.Replace(_T(".apk"),_T("_decompile.log"));
		
		strOutput = RunCmdAndOutput(strCmdLine);

		WriteLogFile(strLogName,strOutput);

		strCmdLine.Empty();
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	if (strOutput.Find("error:") == -1 && strOutput.Find("I: Copying original files") != -1)  //I: Copying original
	{
		strFileName += _T(" OK!");
		//成功，则删除日志文件
		index = _taccess(strLogName,0);
		if (index!=-1)
		{
			DeleteFile(strLogName);
		}
	} 
	else
	{
		strFileName += _T(" Failed，请查看log文件!");
	}

	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName = _T("");
	if (plugData)
		delete plugData;

	return 0;
}


//查看apk信息
unsigned int __stdcall ThreadApkInfor(PVOID pM)
{
	CString fileType;
	CString szUnZipPath;
	CString szCmdLine;
	CString szJarName;
	CString szExtName;
	CString szResult;
	//CString szReturn = _T("");
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	fileType = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		szJarName = strFileName;
		szJarName.Replace(_T(".apk"),_T("_apkinfo.txt"));
		szCmdLine = _T("\"");
		szCmdLine += szModulePath;
		szCmdLine += (_T("aapts.bat\" dump badging \""));
		szCmdLine += strFileName;
		szCmdLine += (_T("\" > \"")+szJarName);
		szCmdLine += _T("\"");
		szResult =RunCmdAndOutput(szCmdLine);
		if (szResult.Find(_T("ERROR")) == -1)
		{
			GetApkFileInfo(szJarName);
			szCmdLine.Empty();
			szJarName.Empty();
		}
		else
		{
			szResult = _T("查看APK信息失败");
			ShowTips(_T("网易易盾加固提醒您"), szResult);
			szCmdLine.Empty();
			szJarName.Empty();
			if (plugData)
				delete plugData;
			return 0;
		}
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	strFileName += _T(" OK!");
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName = _T("");
	if (plugData)
		delete plugData;

	return 0;
}

//Manifest->txt  单文件
unsigned int __stdcall ThreadMani2TxtSignal(PVOID pM)
{
	CString fileType;
	CString szUnZipPath;
	CString szCmdLine;
	CString szJarName;
	CString szExtName;
	list<CString> fileName;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	fileType = strFileName;
	szUnZipPath = strFileName;
	szExtName = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);
	szExtName = szExtName.Left(index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		index = szUnZipPath.ReverseFind(_T('\\'));
		szUnZipPath = szUnZipPath.Left(index);
		szExtName.Delete(0,index+1);

		CZipFunction::ExtractApkToDir( strFileName, szUnZipPath,szExtName,fileName,_T("AndroidManifest.xml"));
		for(list<CString>::iterator it = fileName.begin(); it != fileName.end(); ++it)  
		{  
			//szJarName = szExtName;
			//szJarName += _T('_');
			//szJarName += *it;
			szJarName = *it;
			szJarName.Replace(_T(".xml"),_T(".txt"));
			szCmdLine = _T("\"");
			szCmdLine += szModulePath;
			szCmdLine += (_T("AXMLPrinter.bat\" \"") + szUnZipPath + _T('\\') + *it + _T("\" > \"") + szUnZipPath + _T('\\') + szJarName);
			szCmdLine += _T("\"");

			RunCmdFile(szCmdLine);

	
			index = _taccess(szUnZipPath+_T('\\')+szJarName,0);
			if (index!=-1)
			{
				//continue;
				ShellExecute(NULL,NULL,(_T("\"")+szUnZipPath+_T('\\')+szJarName+_T("\"")),NULL,NULL,SW_SHOW);
			}

			
			szCmdLine.Empty();
			szJarName.Empty();
			szExtName.Empty();
			//Sleep(500);
			index = _taccess(szUnZipPath+_T('\\')+*it,0);
			if (index!=-1)
			{
				DeleteFile(szUnZipPath+_T('\\')+*it);
			}
			
		}  
		fileName.clear();
	}
	else if (!fileType.CompareNoCase(_T(".xml")))
	{
		szJarName = strFileName;
		szJarName.Replace(_T(".xml"),_T(".txt"));
		szCmdLine = _T("\"");
		szCmdLine += szModulePath;
		szCmdLine += _T("AXMLPrinter.bat\" \"");
		szCmdLine += strFileName;
		szCmdLine += _T("\" > \"") + szJarName;
		szCmdLine += _T("\"");

		RunCmdFile(szCmdLine);

		index = _taccess(szJarName,0);
		if (index!=-1)
		{
			//continue;
			ShellExecute(NULL,NULL,(_T("\"")+szJarName+_T("\"")),NULL,NULL,SW_SHOW);
		}

		szCmdLine.Empty();
		szJarName.Empty();
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"),NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	strFileName += _T(" OK!");
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName = _T("");
	if (plugData)
		delete plugData;

	return 0;
}


//Manifest->txt  批量处理
unsigned int __stdcall ThreadMani2Txt(PVOID pM)
{
	CString fileType;
	CString szUnZipPath;
	CString szCmdLine;
	CString szJarName;
	CString szExtName;
	list<CString> fileName;
	int index = 0;
	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	fileType = strFileName;
	szUnZipPath = strFileName;
	szExtName = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);
	szExtName = szExtName.Left(index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		index = szUnZipPath.ReverseFind(_T('\\'));
		szUnZipPath = szUnZipPath.Left(index);
		szExtName.Delete(0,index+1);

		CZipFunction::ExtractApkToDir( strFileName, szUnZipPath,szExtName,fileName,_T("AndroidManifest.xml"));
		for(list<CString>::iterator it = fileName.begin(); it != fileName.end(); ++it)  
		{  
			//szJarName = szExtName;
			//szJarName += _T('_');
			//szJarName += *it;
			szJarName = *it;
			szJarName.Replace(_T(".xml"),_T(".txt"));
			szCmdLine = _T("\"");
			szCmdLine += szModulePath;
			szCmdLine += (_T("AXMLPrinter.bat\" \"")+szUnZipPath+_T('\\')+*it+_T("\" > \"")+szUnZipPath+_T('\\')+szJarName);
			szCmdLine += _T("\"");

			RunCmdFile(szCmdLine);
			szCmdLine.Empty();
			szJarName.Empty();
			szExtName.Empty();
			//Sleep(500);
			index = _taccess(szUnZipPath+_T('\\')+*it,0);
			if (index!=-1)
			{
				DeleteFile(szUnZipPath+_T('\\')+*it);
			}
		}  
		fileName.clear();
	}
	else if (!fileType.CompareNoCase(_T(".xml")))
	{
		szJarName = strFileName;
		szJarName.Replace(_T(".xml"),_T(".txt"));
		szCmdLine = _T("\"");
		szCmdLine += szModulePath;
		szCmdLine += _T("AXMLPrinter.bat\" \"");
		szCmdLine += strFileName;
		szCmdLine += _T("\" > \"")+szJarName;
		szCmdLine += _T("\"");

		RunCmdFile(szCmdLine);
		szCmdLine.Empty();
		szJarName.Empty();
	}
	else
	{
		MessageBox(NULL, _T("请选择正确的文件"), NULL, MB_OK);
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	strFileName += _T(" OK!");
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName = _T("");
	if (plugData)
		delete plugData;

	return 0;
}

//Dex->jar线程
unsigned int __stdcall ThreadDex2Jar(PVOID pM)  
{  
	CString fileType;
	CString strUnZipPath;
	CString strCmdLine;
	CString strJarName;
	CString strExtName;
	//CString szLogName = _T("");
	list<CString> fileName;
	int index = 0;

	if (!pM)
		return -1;

	ThreadData* plugData = (ThreadData*)pM;

	CString strFileName = plugData->strFilePath;

	fileType = strFileName;
	strUnZipPath = strFileName;
	strExtName = strFileName;
	index = fileType.ReverseFind(_T('.'));
	fileType.Delete(0,index);
	strExtName = strExtName.Left(index);

	if(!fileType.CompareNoCase(_T(".apk")))
	{
		index = strUnZipPath.ReverseFind(_T('\\'));
		strUnZipPath = strUnZipPath.Left(index);
		strExtName.Delete(0,index+1);
		strJarName = strExtName;
		strJarName += _T('_');
		CZipFunction::ExtractApkToDir( strFileName, strUnZipPath,strExtName,fileName,_T(".dex"));
		for(list<CString>::iterator it = fileName.begin(); it != fileName.end(); ++it)  
		{  
			strJarName += *it;
			strJarName.Replace(_T(".dex"),_T(".jar"));
			strCmdLine = _T("\"");
			strCmdLine += szModulePath;
			strCmdLine += (_T("dex2jar-2.0\\d2j-dex2jar.bat\" -f \"")+strUnZipPath+_T('\\')+*it+_T("\" -o \"")+strUnZipPath+_T('\\')+strJarName);
			strCmdLine += _T("\"");
			RunCmdFile(strCmdLine);
			strCmdLine = _T("\"");
			strCmdLine += szModulePath;
			index = _taccess(strUnZipPath+_T('\\')+strJarName,0);
			if (index==-1)
			{
				continue;
			}
			strCmdLine += (_T("jdgui\\jd-gui.exe\" \"")+strUnZipPath+_T('\\')+strJarName);
			strCmdLine += _T("\"");
			USES_CONVERSION;
			LPCSTR lpstr=(LPCSTR)T2A(strCmdLine);
			WinExec(lpstr,SW_SHOW);
			//Sleep(100);
			strCmdLine.Empty();
			strJarName.Empty();
			strJarName = strExtName;
			strJarName += _T('_');

			index = _taccess(strUnZipPath+_T('\\')+*it,0);
			if (index!=-1)
			{
				DeleteFile(strUnZipPath+_T('\\')+*it);
			}
		}  
		strExtName.Empty();
		fileName.clear();
	}
	else if (!fileType.CompareNoCase(_T(".dex")))
	{
		strJarName = strFileName;
		strJarName.Replace(_T(".dex"),_T(".jar"));
		strCmdLine = _T("\"");
		strCmdLine += szModulePath;
		strCmdLine += (_T("dex2jar-2.0\\d2j-dex2jar.bat\" -f \""));
		strCmdLine += strFileName;
		strCmdLine += _T("\" -o \"")+strJarName;
		strCmdLine += _T("\"");

		RunCmdFile(strCmdLine);

		strCmdLine = _T("\"");
		strCmdLine += szModulePath;
		index = _taccess(strJarName,0);
		if (index!=-1)
		{
			//continue;
			strCmdLine += (_T("jdgui\\jd-gui.exe\" \"")+strJarName);
			strCmdLine += _T("\"");

			USES_CONVERSION;
			LPCSTR lpstr=(LPCSTR)T2A(strCmdLine);
			WinExec(lpstr,SW_SHOW);
		}
		

		strCmdLine.Empty();
		strJarName.Empty();
	}
	else
	{
		MessageBox(NULL,_T("请选择正确的文件"), NULL,MB_OK);
		//ShowTips(_T("网易易盾加固提醒您，任务完成"), _T("请选择正确的文件"));
		if (plugData)
			delete plugData;

		return 0;
	}

	index = strFileName.ReverseFind(_T('\\'));
	strFileName.Delete(0,index+1);

	strFileName += _T(" OK!");
	ShowTips(_T("网易易盾加固提醒您"),strFileName);
	strFileName.Empty();
	if (plugData)
		delete plugData;

	return 0;  
}  

//About线程
unsigned int __stdcall ThreadAbout(PVOID pM)
{
	CString strCmdLine;

	strCmdLine = szModulePath;
	strCmdLine += _T("about.bat");
	
	if (_taccess(strCmdLine, 0) == 0)
	{
		//MessageBox(NULL,strCmdLine,NULL,0);
		ShellExecute(NULL, _T("open"), strCmdLine, _T(""), NULL, SW_SHOW);
	}
	
	return 0;
}