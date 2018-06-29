#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#define WINAPI _stdcall
#define WM_SOCKET    WM_USER+400

// CMonitor �Ի���

class CMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CMonitor)

public:
	CMonitor(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMonitor();
	SOCKET socket_server;		//���Ӷ˵�socket
	SOCKET socket_client;		//���ӵ����Ӷ˵ı����Ӷ˵�socket
	bool has_client;			//�Ƿ����б����Ӷ����ӵı�־
	CWnd *pw;
	int ww,wh;					//���ڿ��
	CImage Img;					//��ʾ�������
	bool is_recv;
	char Buff[1024];
	IN_ADDR ip_client;

// �Ի�������
	enum { IDD = IDD_MONITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_link;
	afx_msg LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedStart();
	virtual BOOL OnInitDialog();
	void showImage();
};
