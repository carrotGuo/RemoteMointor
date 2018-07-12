#pragma once
#include "afxcmn.h"
#define WM_SOCKET    WM_USER+500
#define WINAPI _stdcall

// CClient 对话框

class CClient : public CDialogEx
{
	DECLARE_DYNAMIC(CClient)

public:
	CClient(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CClient();
	bool link_flag;				//连接监控端成功的标志
	bool send_flag;				//计时发送一个文件结束标志
	int ww,wh;						//屏幕宽度和高度
	CImage Img;
	bool load_flag;
	unsigned long long old_size;	//上一次上传文件的大小
	//int image_index;			//当前发送到第几张图片

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	SOCKET socket_client;			//被监控端socket
	DECLARE_MESSAGE_MAP()
public:
	DWORD m_ip;
	afx_msg LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedConnect();
	void CaptureMultiframe();
	afx_msg void OnBnClickedStartmonitor();
	afx_msg void OnDestroy();
	static DWORD WINAPI sendImg(LPVOID lpParameter);
	afx_msg void OnBnClickedStop();
};
