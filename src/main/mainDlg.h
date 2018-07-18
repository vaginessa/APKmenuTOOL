// mainDlg.h : 头文件
//

#pragma once


// CmainDlg 对话框
class CmainDlg : public CDialog
{
// 构造
public:
	CmainDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MAIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CBrush m_brush;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void RestartExploreOld();
	BOOL RunCMD(LPCTSTR lpszCmdLine, DWORD dwMilliseconds = INFINITE );
	BOOL CreateInstallPath();
	BOOL CreateDirRecursively(const CString&strRoot, const CString&strSubDir);
	void PathWithSlash(CString&strPath);
	BOOL TraverseFolderAndMFile(LPCTSTR lpSrcPath, LPCTSTR lpszDesPath);
	void DeleteDirectory(LPCTSTR lpszDirPath);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();

	void LaunchExplorer();
	void RestartExplore();
	void GetErrorMsg(CString & outPut);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
