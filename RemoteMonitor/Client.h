#pragma once
#include "afxcmn.h"
#define WM_SOCKET    WM_USER+500
#define WINAPI _stdcall

// CClient �Ի���

class CClient : public CDialogEx
{
	DECLARE_DYNAMIC(CClient)

public:
	CClient(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CClient();
	bool link_flag;				//���Ӽ�ض˳ɹ��ı�־
	bool send_flag;				//��ʱ����һ���ļ�������־
	int ww,wh;						//��Ļ��Ⱥ͸߶�
	CImage Img;
	bool load_flag;
	unsigned long long old_size;	//��һ���ϴ��ļ��Ĵ�С
	//int image_index;			//��ǰ���͵��ڼ���ͼƬ

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	SOCKET socket_client;			//����ض�socket
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
