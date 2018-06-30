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
				int can_send = 1;
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
	has_client = false;
	is_recv = false;

	//��ȡ����Picture�����С
	pw = this->GetDlgItem(IDC_IMAGE);
	CRect rect;
	pw->GetClientRect(&rect);
	ww = rect.Width();
	wh = rect.Height();

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
