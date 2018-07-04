// Client.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteMonitor.h"
#include "Client.h"
#include "afxdialogex.h"


// CClient �Ի���

IMPLEMENT_DYNAMIC(CClient, CDialogEx)

CClient::CClient(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClient::IDD, pParent)
	, m_ip(0)
{

}

CClient::~CClient()
{
}

void CClient::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IP, m_ip);
}


BEGIN_MESSAGE_MAP(CClient, CDialogEx)
		ON_MESSAGE(WM_SOCKET,OnSocket)
		ON_BN_CLICKED(IDC_CONNECT, &CClient::OnBnClickedConnect)
		ON_BN_CLICKED(IDC_STARTMONITOR, &CClient::OnBnClickedStartmonitor)
		ON_WM_DESTROY()
		ON_BN_CLICKED(IDC_STOP, &CClient::OnBnClickedStop)
END_MESSAGE_MAP()


// CClient ��Ϣ�������

LRESULT CClient::OnSocket(WPARAM wParam, LPARAM lParam){
	switch(lParam){
		case FD_READ:{
			bool can_send = false;
			byte buff;
			recv(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL);
			can_send = (bool)buff;
			if(can_send){
				//sendImg();
				HANDLE hThread = CreateThread(NULL,0,sendImg,this,0,NULL);
				//�رոý����߳̾�����ͷ����ü���
				CloseHandle(hThread);
			}
			break;		 
		}
	}
	return 1;
}


BOOL CClient::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	link_flag = false;
	load_flag = false;

	CString file = "Screen";
	if(!PathIsDirectory(file)){
		CreateDirectory(file,NULL);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

/**
*	���Ӽ�ض�
*/
void CClient::OnBnClickedConnect()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
		// TODO: �ڴ���ӿؼ�֪ͨ����������
	////========����Winsock��̬���ӿ�======	
	WORD	version = MAKEWORD(2,0);
	WSADATA wsadata;
	if(WSAStartup(version,&wsadata))
	{
		MessageBox("����Winsockʧ��");
		return;
	}

//=============�����׽���======================
	socket_client = socket(AF_INET,SOCK_STREAM,NULL);
	if(INVALID_SOCKET == socket_client)
	{
		MessageBox("�׽��ִ���ʧ��");
		return;
	}
//==============��ȡ��������ַ===================

	UpdateData(true);
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.S_un.S_addr =htonl(m_ip);//inet_addr
	if(connect(socket_client,(sockaddr *)&addr,sizeof(sockaddr)))
	{
		MessageBox("�����������������ʧ��");
		return;
	}

	if( WSAAsyncSelect(socket_client,this->m_hWnd,WM_SOCKET,FD_READ))
	{
		MessageBox("�첽���ó���");
		return;
	}
	send_flag=true;
	AfxMessageBox(_T("���ӷ������ɹ���"));

}

void CClient::CaptureMultiframe(){
	if (!Img.IsNull()) {
		Img.Destroy();
	}
	CDC *pDC;	//��ĻDC
	CString filename;
	filename = "Screen\\ImageOne.jpg";
	CFile file;
	CWnd *pWnd;
	pWnd = this->GetDesktopWindow();
	pDC = pWnd->GetDC();	//��ȡ��ǰ������Ļ��DC
	int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);	//��ȡ��ɫģʽ
	int width = pDC->GetDeviceCaps(HORZRES);	//��ȡˮƽ�ֱ���
	int height = pDC->GetDeviceCaps(VERTRES);	//��ȡ��ֱ�ֱ���
	CDC memDC;			//��������Ļ�����ڴ��DC
	memDC.CreateCompatibleDC(pDC);
	CBitmap memBitmap,*oldmemBitmap;		//��������Ļ���ݵ�bitmap
	memBitmap.CreateCompatibleBitmap(pDC,width,height);
	oldmemBitmap = memDC.SelectObject(&memBitmap);
	memDC.BitBlt(0,0,width,height,pDC,0,0,SRCCOPY);	//������Ļͼ���ڴ�DC

	//���´��뱣��memDC�е�λͼ���ļ���
	//������
	HICON hcur;
	CPoint pt;
	hcur = (HICON)::LoadCursorA(NULL,IDC_ARROW);
	GetCursorPos(&pt);
	memDC.DrawIcon(pt.x,pt.y,hcur);
	Img.Attach((HBITMAP)memBitmap.GetSafeHandle());
	Img.Save(filename);
	Img.Detach();
	memBitmap.DeleteObject();
	memDC.DeleteDC();
	pWnd->ReleaseDC(pDC);
	Img.Destroy();
}


void CClient::OnBnClickedStartmonitor()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	HANDLE hThread = CreateThread(NULL,0,sendImg,this,0,NULL);
	//�رոý����߳̾�����ͷ����ü���
	CloseHandle(hThread);
}

/**
*	��ȡ��Ļ���ҷ���ͼƬ
*/
DWORD WINAPI CClient::sendImg(LPVOID lpParameter){
	CClient *pThis = (CClient*)lpParameter;
	pThis->CaptureMultiframe();						//�Ƚ�ȡ��ǰ��Ļ

	CString filename = "Screen\\ImageOne.jpg";	//���͵��ļ�����
	HANDLE hFile;
	unsigned long long file_size = 0;
	char Buffer[512];							//��������С
	DWORD dwNumberOfBytesRead;					//��ȡ�ļ����ֽ���
	
	hFile = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);	//�����ļ����
	file_size = GetFileSize(hFile,NULL);		//��ȡ�ļ���С
	send(pThis->socket_client,(char*)&file_size,sizeof(unsigned long long)+1,NULL);									//�ȷ����ļ���С
	//�����ļ�(�������ֽڴ���0��ѭ������)
	do{
		::ReadFile(hFile,Buffer,sizeof(Buffer),&dwNumberOfBytesRead,NULL);										//���ļ�ĳһ���ֵ�dwNumberOfBytesRead
		::send(pThis->socket_client,Buffer,dwNumberOfBytesRead,0);														//�����ļ�
	}while(dwNumberOfBytesRead);
	CloseHandle(hFile);
	return 0;
}


void CClient::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
}


void CClient::OnBnClickedStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	closesocket(socket_client);
}
