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
	IN_ADDR ip_client;
	CDC *pdc;

	BOOL bFullScreen;  
    CRect rectFullScreen;  
    WINDOWPLACEMENT m_struOldWndpl;//�ṹ�а������йش�������Ļ��λ�õ���Ϣ  
	WINDOWPLACEMENT m_struOldWndpPic;//PICTURE�ؼ�����Ļ��λ�õ���Ϣ  
	CBitmap bmp1,bmp2;

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
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
