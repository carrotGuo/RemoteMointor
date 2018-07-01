// Monitor.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteMonitor.h"
#include "Monitor.h"
#include "afxdialogex.h"


// CMonitor �Ի���

IMPLEMENT_DYNAMIC(CMonitor, CDialogEx)

CMonitor::CMonitor(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMonitor::IDD, pParent)
	, m_link(_T("���Ӷ�δ����"))
{

}

CMonitor::~CMonitor()
{
}

void CMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATE, m_link);
}


BEGIN_MESSAGE_MAP(CMonitor, CDialogEx)
	ON_BN_CLICKED(IDC_START, &CMonitor::OnBnClickedStart)
	ON_MESSAGE(WM_SOCKET,OnSocket)
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


// CMonitor ��Ϣ�������

/**
*	���Ӷ��뱻���Ӷ˵�SOCKETͨ��
*/
LRESULT CMonitor::OnSocket(WPARAM wParam, LPARAM lParam){
	switch(lParam){
		case FD_ACCEPT:{
			if(has_client){
				break;
			}
			SOCKADDR_IN addr;
			int len = sizeof(SOCKADDR_IN);
			socket_client = accept(socket_server,(sockaddr *)&addr,&len);
			if(INVALID_SOCKET == socket_client){
				MessageBox("�ͻ����׽��ִ�������");
				return -1;
			} else {
				//��ȡ�Է�IP
				m_link = "��";
				m_link += inet_ntoa(addr.sin_addr);
				m_link += "���ӳɹ�";
				UpdateData(false);
				has_client = true;
				break;
			}
		}
		case FD_READ:{
			if(!is_recv){
				is_recv = true;
				bool can_send = true;
				byte buff = (unsigned char)can_send;
				unsigned long long file_size = 0;
				//�����ļ���С�����浽file_size
				if(!recv(socket_client,(char*)&file_size,sizeof(unsigned long long)+1,NULL)){
					MessageBox("�����������ļ���Сʱ����");
				}

				//�ļ���С����0�Ŷ��ļ����ұ���
				if(file_size>0){
					DWORD dwNumberOfBytesRecv = 0;		//���յ����ֽ���
					DWORD dwCountOfBytesRecv = 0;		//�ѽ����ļ���С
					char Buffer[1024];
					CString filename = "Recv\\ImageOne.jpg";
					HANDLE hFile = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
					//ѭ�����ļ�
					do{
						//�����ļ�
						dwNumberOfBytesRecv = ::recv(socket_client,Buffer,sizeof(Buffer),0);
						//�����ļ�
						::WriteFile(hFile,Buffer,dwNumberOfBytesRecv,&dwNumberOfBytesRecv,false);
						dwCountOfBytesRecv += dwNumberOfBytesRecv;
					}while(file_size-dwCountOfBytesRecv);
					CloseHandle(hFile);
					//��ͼƬ��ʾ���ؼ�
					showImage();
					if(SOCKET_ERROR == send(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL)){
						MessageBox("��Ϣ���ʹ���");
						return 1;
					}
				}
				is_recv = false;
			}
			break;
		}
	}
	return 1;
}

void CMonitor::showImage(){

	pw = this->GetDlgItem(IDC_IMAGE);
	CString filename = "RECV\\ImageOne.jpg";
	if(Img!=NULL){
		Img.Destroy();
	}
	Img.Load(filename);
	//CDC *pdc;
	pdc = pw->GetDC();
	pdc->SetStretchBltMode(COLORONCOLOR);
	Img.Draw(pdc->m_hDC,0,0,ww,wh);
	pw->ReleaseDC(pdc);		//�ͷ�PDC
	Img.Destroy();
}

/**
*	�������Ӷ�
*/
void CMonitor::OnBnClickedStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	////===========�������ӿ�=============
	WORD version = MAKEWORD(2,0);
	WSADATA wsadata;
	if(WSAStartup(version,&wsadata)){
		MessageBox("����Winsock dll ʧ��");
		return;
	}

	////===========�����׽���=============
	socket_server = socket(AF_INET,SOCK_STREAM,NULL);
	if(INVALID_SOCKET == socket_server){
		MessageBox("�׽��ִ���ʧ��");
		return;
	}

	////============�Զ���ȡ����IP==============
	char hostname[20] = "";
	if(gethostname(hostname,20)){
		MessageBox("��������ȡʧ��");
		return;
	}
	hostent *htent = gethostbyname(hostname);
	if(htent == NULL){
		MessageBox("����IP��ȡʧ��");
		return;
	}
	LPSTR lpAddr = htent->h_addr_list[0];
	IN_ADDR inAddr;
	memmove(&inAddr,lpAddr,4);		//4���ֽڵ�IP��ַ
	char *ipAddress = inet_ntoa(inAddr);		//�������ַתΪ������ַ��ʽ
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));		//��ʼ�������׽��ֵ�ַ��ȫ��0
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.S_un.S_addr = inet_addr(ipAddress);

	////=============���׽���==============
	if(bind(socket_server,(sockaddr *)&addr,sizeof(sockaddr))){
		MessageBox("��IP��port����");
		return;
	}

	////==============�����������==============
	if(listen(socket_server,SOMAXCONN)){
		MessageBox("��������");
		return;
	}

	////=========�����첽�׽��������׽��ִ�����Ϣӳ��==========
	if( WSAAsyncSelect(socket_server,this->m_hWnd,WM_SOCKET,FD_ACCEPT | FD_READ))
	{
		MessageBox("�첽���ó���");
		return;
	}

	m_link = "��������״̬�У��ȴ������Ӷ�����";
	UpdateData(false);
}


BOOL CMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	
	//����ȫ�� ���ǿؼ�û�б仯
	//int cxScreen,cyScreen; 
	//cxScreen=GetSystemMetrics(SM_CXSCREEN);
	//cyScreen=GetSystemMetrics(SM_CYSCREEN);
	//SetWindowPos(&wndTopMost,0,0,cxScreen,cyScreen,SWP_SHOWWINDOW);

	has_client = false;
	is_recv = false;

	//��ȡ����Picture�����С
	pw = this->GetDlgItem(IDC_IMAGE);
	CRect rect;
	pw->GetClientRect(&rect);
	ww = rect.Width();
	wh = rect.Height();

	bFullScreen = false;
	bmp1.LoadBitmapA(IDB_BITMAP2);
	bmp2.LoadBitmapA(IDB_BITMAP3);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

//BOOL CMonitor::PreTranslateMessage(MSG* pMsg)
//{
//	// TODO: �ڴ����ר�ô����/����û���
//	if (WM_LBUTTONDBLCLK == pMsg->message) {
//		if (!bFullScreen) {  
//			bFullScreen = true;  
//			//��ȡϵͳ��Ļ���  
//			int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);  
//			int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);  
//  
//			//��m_struOldWndpl�õ���ǰ���ڵ���ʾ״̬�ʹ���λ�ã��Թ��˳�ȫ����ʹ��  
//			GetWindowPlacement(&m_struOldWndpl);  
//			GetDlgItem(IDC_IMAGE)->GetWindowPlacement(&m_struOldWndpPic);  
//      
//			//���������ȫ����ʾ�ͻ�����Ӧ�����õĴ��ڴ�С����ҪΪ�˽�����Ҫ��ʾ�Ĵ���߿�Ȳ����ų�����Ļ��  
//			CRect rectWholeDlg;  
//			CRect rectClient;  
//			GetWindowRect(&rectWholeDlg);//�õ���ǰ������ܵ��������Ļ������  
//			RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);//�õ��ͻ�����������  
//			ClientToScreen(&rectClient);//���ͻ�����Դ��������תΪ�����Ļ����  
//			//GetDlgItem(IDC_STATIC_PICSHOW)->GetWindowRect(rectClient);//�õ�PICTURE�ؼ�����  
//  
//			rectFullScreen.left = rectWholeDlg.left - rectClient.left;  
//			rectFullScreen.top = rectWholeDlg.top - rectClient.top;  
//			rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;  
//			rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;  
//  
//			//���ô��ڶ��������Ϊȫ������׼��������ȫ��״̬  
//			WINDOWPLACEMENT struWndpl;  
//			struWndpl.length = sizeof(WINDOWPLACEMENT);   
//			struWndpl.flags = 0;  
//			struWndpl.showCmd = SW_SHOWNORMAL;  
//			struWndpl.rcNormalPosition = rectFullScreen;  
//			SetWindowPlacement(&struWndpl);//�ú�������ָ�����ڵ���ʾ״̬����ʾ��Сλ�õȣ������Ǹó�����Ϊ��Ҫ�ĺ���  
//  
//			//��PICTURE�ؼ���������Ϊȫ����С  
//			GetDlgItem(IDC_IMAGE)->MoveWindow(CRect(0, 0, g_iCurScreenWidth, g_iCurScreenHeight));  
//		} else {  
//			GetDlgItem(IDC_IMAGE)->SetWindowPlacement(&m_struOldWndpPic);  
//			SetWindowPlacement(&m_struOldWndpl);  
//			bFullScreen = false;  
//		}  
//	}
//	return CDialogEx::PreTranslateMessage(pMsg);
//}


void CMonitor::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (bFullScreen)  {  
        lpMMI->ptMaxSize.x = rectFullScreen.Width();  
        lpMMI->ptMaxSize.y = rectFullScreen.Height();  
        lpMMI->ptMaxPosition.x = rectFullScreen.left;  
        lpMMI->ptMaxPosition.y = rectFullScreen.top;  
        lpMMI->ptMaxTrackSize.x = rectFullScreen.Width();  
        lpMMI->ptMaxTrackSize.y = rectFullScreen.Height();  
    }  
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CMonitor::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (!bFullScreen)  
	{  
		bFullScreen = true;  
  
		//��ȡϵͳ��Ļ���  
		int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);  
		int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);  
  
		//��m_struOldWndpl�õ���ǰ���ڵ���ʾ״̬�ʹ���λ�ã��Թ��˳�ȫ����ʹ��  
		GetWindowPlacement(&m_struOldWndpl);  
		GetDlgItem(IDC_IMAGE)->GetWindowPlacement(&m_struOldWndpPic);  
      
		//���������ȫ����ʾ�ͻ�����Ӧ�����õĴ��ڴ�С����ҪΪ�˽�����Ҫ��ʾ�Ĵ���߿�Ȳ����ų�����Ļ��  
		CRect rectWholeDlg;  
		CRect rectClient;  
		GetWindowRect(&rectWholeDlg);//�õ���ǰ������ܵ��������Ļ������  
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);//�õ��ͻ�����������  
		ClientToScreen(&rectClient);//���ͻ�����Դ��������תΪ�����Ļ����  
		//GetDlgItem(IDC_IMAGE)->GetWindowRect(rectClient);//�õ�PICTURE�ؼ�����  
  
		rectFullScreen.left = rectWholeDlg.left - rectClient.left;  
		rectFullScreen.top = rectWholeDlg.top - rectClient.top;  
		rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;  
		rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;  
  
		//���ô��ڶ��������Ϊȫ������׼��������ȫ��״̬  
		WINDOWPLACEMENT struWndpl;  
		struWndpl.length = sizeof(WINDOWPLACEMENT);   
		struWndpl.flags = 0;  
		struWndpl.showCmd = SW_SHOWNORMAL;  
		struWndpl.rcNormalPosition = rectFullScreen;  
		SetWindowPlacement(&struWndpl);//�ú�������ָ�����ڵ���ʾ״̬����ʾ��Сλ�õȣ������Ǹó�����Ϊ��Ҫ�ĺ���  
  
		//���ؿؼ�
        GetDlgItem(IDC_STATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_START)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);

		//��PICTURE�ؼ���������Ϊȫ����С  
		GetDlgItem(IDC_IMAGE)->MoveWindow(CRect(0, 0, g_iCurScreenWidth, g_iCurScreenHeight));  
		CWnd *pWnd;
		pWnd = this->GetDlgItem(IDC_IMAGE);
		CBrush br;
		br.CreatePatternBrush(&bmp1);
		CDC *pdc;
		pdc = pWnd->GetDC();
		CRect rect;
		this->GetClientRect(&rect);
		pdc->FillRect(&rect,&br);
	}  
	else  
	{  
		//��ʾ�ؼ�
        GetDlgItem(IDC_STATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_START)->ShowWindow(SW_SHOW);
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IMAGE)->SetWindowPlacement(&m_struOldWndpPic);  
		SetWindowPlacement(&m_struOldWndpl);  
		CWnd *pWnd;
		pWnd = this->GetDlgItem(IDC_IMAGE);
		CBrush br;
		br.CreatePatternBrush(&bmp2);
		CDC *pdc;
		pdc = pWnd->GetDC();
		CRect rect;
		pWnd->GetClientRect(&rect);
		pdc->FillRect(&rect,&br);
		bFullScreen = false;  
	}  
	CDialogEx::OnLButtonDblClk(nFlags, point);
}
