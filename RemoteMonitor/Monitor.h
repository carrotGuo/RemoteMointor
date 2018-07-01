#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#define WINAPI _stdcall
#define WM_SOCKET    WM_USER+400

// CMonitor 对话框

class CMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CMonitor)

public:
	CMonitor(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMonitor();
	SOCKET socket_server;		//监视端的socket
	SOCKET socket_client;		//连接到监视端的被监视端的socket
	bool has_client;			//是否已有被监视端连接的标志
	CWnd *pw;
	int ww,wh;					//窗口宽高
	CImage Img;					//显示监控区域
	bool is_recv;
	IN_ADDR ip_client;
	CDC *pdc;

	BOOL bFullScreen;  
    CRect rectFullScreen;  
    WINDOWPLACEMENT m_struOldWndpl;//结构中包含了有关窗口在屏幕上位置的信息  
	WINDOWPLACEMENT m_struOldWndpPic;//PICTURE控件在屏幕上位置的信息  
	CBitmap bmp1,bmp2;

// 对话框数据
	enum { IDD = IDD_MONITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_link;
	afx_msg LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedStart();
	virtual BOOL OnInitDialog();
	void showImage();
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
