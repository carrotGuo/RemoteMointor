// Client.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteMonitor.h"
#include "Client.h"
#include "afxdialogex.h"


// CClient 对话框

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


// CClient 消息处理程序

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
				//关闭该接收线程句柄，释放引用计数
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

	// TODO:  在此添加额外的初始化
	link_flag = false;
	load_flag = false;

	CString file = "Screen";
	if(!PathIsDirectory(file)){
		CreateDirectory(file,NULL);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

/**
*	连接监控端
*/
void CClient::OnBnClickedConnect()
{
	// TODO: 在此添加控件通知处理程序代码
		// TODO: 在此添加控件通知处理程序代码
	////========加载Winsock动态链接库======	
	WORD	version = MAKEWORD(2,0);
	WSADATA wsadata;
	if(WSAStartup(version,&wsadata))
	{
		MessageBox("加载Winsock失败");
		return;
	}

//=============创建套接字======================
	socket_client = socket(AF_INET,SOCK_STREAM,NULL);
	if(INVALID_SOCKET == socket_client)
	{
		MessageBox("套接字创建失败");
		return;
	}
//==============获取服务器地址===================

	UpdateData(true);
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.S_un.S_addr =htonl(m_ip);//inet_addr
	if(connect(socket_client,(sockaddr *)&addr,sizeof(sockaddr)))
	{
		MessageBox("尝试与服务器的连接失败");
		return;
	}

	if( WSAAsyncSelect(socket_client,this->m_hWnd,WM_SOCKET,FD_READ))
	{
		MessageBox("异步设置出错");
		return;
	}
	send_flag=true;
	AfxMessageBox(_T("连接服务器成功！"));

}

void CClient::CaptureMultiframe(){
	if (!Img.IsNull()) {
		Img.Destroy();
	}
	CDC *pDC;	//屏幕DC
	CString filename;
	filename = "Screen\\ImageOne.jpg";
	CFile file;
	CWnd *pWnd;
	pWnd = this->GetDesktopWindow();
	pDC = pWnd->GetDC();	//获取当前整个屏幕的DC
	int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);	//获取颜色模式
	int width = pDC->GetDeviceCaps(HORZRES);	//获取水平分辨率
	int height = pDC->GetDeviceCaps(VERTRES);	//获取垂直分辨率
	CDC memDC;			//建立与屏幕兼容内存的DC
	memDC.CreateCompatibleDC(pDC);
	CBitmap memBitmap,*oldmemBitmap;		//建立与屏幕兼容的bitmap
	memBitmap.CreateCompatibleBitmap(pDC,width,height);
	oldmemBitmap = memDC.SelectObject(&memBitmap);
	memDC.BitBlt(0,0,width,height,pDC,0,0,SRCCOPY);	//复制屏幕图像到内存DC

	//以下代码保存memDC中的位图到文件中
	//添加鼠标
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
	// TODO: 在此添加控件通知处理程序代码
	HANDLE hThread = CreateThread(NULL,0,sendImg,this,0,NULL);
	//关闭该接收线程句柄，释放引用计数
	CloseHandle(hThread);
}

/**
*	截取屏幕并且发送图片
*/
DWORD WINAPI CClient::sendImg(LPVOID lpParameter){
	CClient *pThis = (CClient*)lpParameter;
	pThis->CaptureMultiframe();						//先截取当前屏幕

	CString filename = "Screen\\ImageOne.jpg";	//发送的文件名称
	HANDLE hFile;
	unsigned long long file_size = 0;
	char Buffer[512];							//缓存区大小
	DWORD dwNumberOfBytesRead;					//读取文件的字节数
	
	hFile = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);	//创建文件句柄
	file_size = GetFileSize(hFile,NULL);		//获取文件大小
	send(pThis->socket_client,(char*)&file_size,sizeof(unsigned long long)+1,NULL);									//先发送文件大小
	//发送文件(读到的字节大于0就循环发送)
	do{
		::ReadFile(hFile,Buffer,sizeof(Buffer),&dwNumberOfBytesRead,NULL);										//读文件某一部分到dwNumberOfBytesRead
		::send(pThis->socket_client,Buffer,dwNumberOfBytesRead,0);														//发送文件
	}while(dwNumberOfBytesRead);
	CloseHandle(hFile);
	return 0;
}


void CClient::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void CClient::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	closesocket(socket_client);
}
