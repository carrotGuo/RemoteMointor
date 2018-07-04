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
	ON_BN_CLICKED(IDC_START_RECORD, &CMonitor::OnBnClickedStartRecord)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_STOP_RECORD, &CMonitor::OnBnClickedStopRecord)
	ON_BN_CLICKED(IDC_PLAY, &CMonitor::OnBnClickedPlay)
END_MESSAGE_MAP()


// CMonitor ��Ϣ�������

/**
*	���Ӷ��뱻���Ӷ˵�SOCKETͨ��
*/
LRESULT CMonitor::OnSocket(WPARAM wParam, LPARAM lParam){
	switch(lParam){
		case FD_ACCEPT:{
			if(has_client || !can_accept){
				MessageBox("�ͻ����޷�����");
				break;
			}
			SOCKADDR_IN addr;
			int len = sizeof(SOCKADDR_IN);
			socket_client = accept(socket_server,(sockaddr *)&addr,&len);
			if(INVALID_SOCKET == socket_client){
				MessageBox("�ͻ����׽��ִ�������");
				this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
				this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
				this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
				return -1;
			} else {
				//��ȡ�Է�IP
				m_link = "�뱻���Ӷ�";
				m_link += inet_ntoa(addr.sin_addr);
				m_link += "���ӳɹ�";
				UpdateData(false);
				has_client = true;
				can_accept = false;
				this->GetDlgItem(IDC_START_RECORD)->EnableWindow(true);
				this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
				this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
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
					char Buffer[512];
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

					//��Ļ¼��
					if(is_record){
						CString index;
						index.Format("%d",record_num);
						CString recordFilename = "Record\\Image"+index+".jpg";
						record_num++;
						if(!CopyFileA(filename,recordFilename,false)){
							MessageBox("¼��ʧ�ܣ�");
						}
						if(record_num>200){
							is_record = false;
							MessageBox("¼�Ƴɹ����������Ű�ť");
							//WSACleanup();				//ж��winsock��̬��
							closesocket(socket_client);
							has_client = false;
							can_accept = true;
							this->GetDlgItem(IDC_START_RECORD)->EnableWindow(true);
							this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
							this->GetDlgItem(IDC_PLAY)->EnableWindow(true);
						}
					}
					if(SOCKET_ERROR == send(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL)){
						send(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL);		//�ٴγ��Է���
						//MessageBox("��Ϣ���ʹ���");
						return 1;
					}
				}
				is_recv = false;
			}
			break;
		}
		case FD_CLOSE :{
			MessageBox("�����Ӷ��ѶϿ�����");
			has_client = false;
			can_accept = true;		//���������ͻ�������
			this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
			this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
			this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
			m_link = "���Ӷ����ڵȴ������Ӷ�����... ...";
			UpdateData(false);
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
	//pw->ReleaseDC(pdc);		//�ͷ�PDC
	ReleaseDC(pdc);
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
	if( WSAAsyncSelect(socket_server,this->m_hWnd,WM_SOCKET,FD_ACCEPT | FD_READ | FD_CLOSE))
	{
		MessageBox("�첽���ó���");
		return;
	}

	m_link = "���Ӷ����������ȴ������Ӷ�����... ...";
	this->GetDlgItem(IDC_START)->EnableWindow(false);		//�����ٴ�����������
	UpdateData(false);
}


BOOL CMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	has_client = false;				//�Ƿ����пͻ�������
	is_recv = false;				//�Ƿ����ڽ����ļ� ������ڽ���ͼƬ  �򲻽�������socket���� ��ֹ���ļ�ʧ��
	is_record = false;				//��Ļ¼�Ʊ�־
	can_accept = true;				//�Ƿ���������ͻ�������(�طŵ�ʱ����Ȼû�пͻ������� ���ǲ����������ͻ�����������  ���Ž����ſ���)
	record_num = 0;					//��ǰ¼���˶�����ͼ
	play_index = 0;					//��ǰ���ŵ��ڼ���ͼ

	//��ȡ����Picture�����С
	pw = this->GetDlgItem(IDC_IMAGE);
	CRect rect;
	pw->GetClientRect(&rect);
	ww = rect.Width();
	wh = rect.Height();
	oww = ww;
	owh = wh;

	bFullScreen = false;
	bmp1.LoadBitmapA(IDB_BITMAP2);
	bmp2.LoadBitmapA(IDB_BITMAP3);

	//�����ļ���
	CString file = "Recv";
	if(!PathIsDirectory(file)){
		CreateDirectory(file,NULL);
	}
	CString record_file = "Record";
		if(!PathIsDirectory(record_file)){
		CreateDirectory(record_file,NULL);
	}

	//��������״̬���ƿؼ��Ƿ�ɵ��
	this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_PLAY)->EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


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
	//ȫ����ʾ
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
		GetDlgItem(IDC_START_RECORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STOP_RECORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PLAY)->ShowWindow(SW_HIDE);

		//��PICTURE�ؼ���������Ϊȫ����С  
		GetDlgItem(IDC_IMAGE)->MoveWindow(CRect(0, 0, g_iCurScreenWidth, g_iCurScreenHeight));  
		ww = g_iCurScreenWidth;
		wh = g_iCurScreenHeight;
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
		GetDlgItem(IDC_START_RECORD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STOP_RECORD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PLAY)->ShowWindow(SW_SHOW);

		GetDlgItem(IDC_IMAGE)->SetWindowPlacement(&m_struOldWndpPic);
		ww = oww;
		wh = owh;
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

/**
*	����¼��
*/
void CMonitor::OnBnClickedStartRecord()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (has_client) {
		DeleteDirectory("Record");			//������ļ���
		CreateDirectory("Record",NULL);		//�ٴδ����ļ��� �������û��ٴ�¼��
		record_num = 0;
		is_record = true;
		this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
		this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(true);
		this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
	} else {
		MessageBox("��ǰδ���ӱ����Ӷˣ��޷�¼��");
	}
}


void CMonitor::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	play_index = record_num;			//�ò���¼�������߳��Զ��˳�  ��ִ��ɾ���ļ�����
	DeleteDirectory("Record"); 
	CDialogEx::OnClose();
}

/**
*	ɾ���ļ��м������ļ�
*/
bool CMonitor::DeleteDirectory( char* DirName){
	HANDLE hFirstFile = NULL; 
	WIN32_FIND_DATA FindData; 

	char currdir[MAX_PATH] = {0};
	sprintf(currdir, "%s\\*.*", DirName);

	hFirstFile = ::FindFirstFile(currdir, &FindData); 
	if( hFirstFile == INVALID_HANDLE_VALUE ) 
	   return false;

	BOOL bRes = true;

	while(bRes) 
	{ 
	   bRes = ::FindNextFile(hFirstFile, &FindData);

	   if( (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) //����Ŀ¼
	   {
		if( !strcmp(FindData.cFileName, ".") || !strcmp(FindData.cFileName, "..") ) //.��..
		 continue;
		else
		{
		 char tmppath[MAX_PATH] = {0};
		 sprintf(tmppath, "%s\\%s", DirName, FindData.cFileName);
    
		 DeleteDirectory(tmppath);
		}
	   }
	   else               //�����ļ�
	   {
		char tmppath[MAX_PATH] = {0};
		sprintf(tmppath, "%s\\%s", DirName, FindData.cFileName);
		::DeleteFile(tmppath);    
	   }
	} 
	::FindClose(hFirstFile);
	if(!RemoveDirectory(DirName))
	{
	   return false ;
	}
	return true;
}

/**
*	����¼��
*/
void CMonitor::OnBnClickedStopRecord()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (has_client) {
		is_record = false;
		//closesocket(socket_client);
		this->GetDlgItem(IDC_START_RECORD)->EnableWindow(true);
		this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
		this->GetDlgItem(IDC_PLAY)->EnableWindow(true);
	} else {
		MessageBox("δ���ӱ����Ӷ�");
	}

}

/**
*	��ʼ����  �Ͽ�socket
*/
void CMonitor::OnBnClickedPlay()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//ʹ���߳�˯�߷�ʽ����
	closesocket(socket_client);
	m_link = "���Ӷ����������ȴ������Ӷ�����... ...";
	UpdateData(false);
	//WSACleanup();				//ж��winsock��̬��
	has_client = false;
	can_accept = false;
	this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
	//this->GetDlgItem(IDC_START)->EnableWindow(true);		//����������������
	play_index = 0;
	HANDLE hThread = CreateThread(NULL,0,Play,this,0,NULL);
	//�رոý����߳̾�����ͷ����ü���
	CloseHandle(hThread);
}

DWORD WINAPI CMonitor::Play(LPVOID lpParameter){
	CMonitor *pThis = (CMonitor*)lpParameter;
	pThis->pw = pThis->GetDlgItem(IDC_IMAGE);
	while(pThis->play_index<pThis->record_num){		//δ���Ž�����һֱ������һ��ͼƬ
		CString index;
		index.Format("%d",pThis->play_index);
		CString recordFilename = "Record\\Image"+index+".jpg";
		if(pThis->Img!=NULL){
			pThis->Img.Destroy();
		}
		pThis->Img.Load(recordFilename);
		//CDC *pdc;
		pThis->pdc = pThis->pw->GetDC();
		pThis->pdc->SetStretchBltMode(COLORONCOLOR);
		pThis->Img.Draw(pThis->pdc->m_hDC,0,0,pThis->ww,pThis->wh);
		//pw->ReleaseDC(pdc);		//�ͷ�PDC
		pThis->ReleaseDC(pThis->pdc);
		pThis->Img.Destroy();
		pThis->play_index++;
		Sleep(100);
	}
	pThis->GetDlgItem(IDC_PLAY)->EnableWindow(true);		//���Ž���  ���԰��ٴβ���
	pThis->can_accept = true;								//�ɽ��������ͻ�������
	return 0;
}


