// Monitor.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteMonitor.h"
#include "Monitor.h"
#include "afxdialogex.h"


// CMonitor 对话框

IMPLEMENT_DYNAMIC(CMonitor, CDialogEx)

CMonitor::CMonitor(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMonitor::IDD, pParent)
	, m_link(_T("监视端未启动"))
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


// CMonitor 消息处理程序

/**
*	监视端与被监视端的SOCKET通信
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
				MessageBox("客户端套接字创建出错");
				return -1;
			} else {
				//获取对方IP
				m_link = "与";
				m_link += inet_ntoa(addr.sin_addr);
				m_link += "连接成功";
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
				//接收文件大小并保存到file_size
				if(!recv(socket_client,(char*)&file_size,sizeof(unsigned long long)+1,NULL)){
					MessageBox("服务器接收文件大小时出错");
				}

				//文件大小大于0才读文件并且保存
				if(file_size>0){
					DWORD dwNumberOfBytesRecv = 0;		//接收到的字节数
					DWORD dwCountOfBytesRecv = 0;		//已接收文件大小
					char Buffer[1024];
					CString filename = "Recv\\ImageOne.jpg";
					HANDLE hFile = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
					//循环读文件
					do{
						//接收文件
						dwNumberOfBytesRecv = ::recv(socket_client,Buffer,sizeof(Buffer),0);
						//保存文件
						::WriteFile(hFile,Buffer,dwNumberOfBytesRecv,&dwNumberOfBytesRecv,false);
						dwCountOfBytesRecv += dwNumberOfBytesRecv;
					}while(file_size-dwCountOfBytesRecv);
					CloseHandle(hFile);
					//将图片显示到控件
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
	pw->ReleaseDC(pdc);		//释放PDC
	Img.Destroy();
}

/**
*	启动监视端
*/
void CMonitor::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	////===========创建链接库=============
	WORD version = MAKEWORD(2,0);
	WSADATA wsadata;
	if(WSAStartup(version,&wsadata)){
		MessageBox("加载Winsock dll 失败");
		return;
	}

	////===========创建套接字=============
	socket_server = socket(AF_INET,SOCK_STREAM,NULL);
	if(INVALID_SOCKET == socket_server){
		MessageBox("套接字创建失败");
		return;
	}

	////============自动获取主机IP==============
	char hostname[20] = "";
	if(gethostname(hostname,20)){
		MessageBox("主机名获取失败");
		return;
	}
	hostent *htent = gethostbyname(hostname);
	if(htent == NULL){
		MessageBox("主机IP获取失败");
		return;
	}
	LPSTR lpAddr = htent->h_addr_list[0];
	IN_ADDR inAddr;
	memmove(&inAddr,lpAddr,4);		//4个字节的IP地址
	char *ipAddress = inet_ntoa(inAddr);		//将网络地址转为主机地址格式
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));		//初始化，将套接字地址域全清0
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.S_un.S_addr = inet_addr(ipAddress);

	////=============绑定套接字==============
	if(bind(socket_server,(sockaddr *)&addr,sizeof(sockaddr))){
		MessageBox("绑定IP和port出错");
		return;
	}

	////==============启动服务监听==============
	if(listen(socket_server,SOMAXCONN)){
		MessageBox("监听出错");
		return;
	}

	////=========设置异步套接字启动套接字窗口消息映射==========
	if( WSAAsyncSelect(socket_server,this->m_hWnd,WM_SOCKET,FD_ACCEPT | FD_READ))
	{
		MessageBox("异步设置出错");
		return;
	}

	m_link = "启动监视状态中，等待被监视端连接";
	UpdateData(false);
}


BOOL CMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	has_client = false;
	is_recv = false;

	//获取窗口Picture区域大小
	pw = this->GetDlgItem(IDC_IMAGE);
	CRect rect;
	pw->GetClientRect(&rect);
	ww = rect.Width();
	wh = rect.Height();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
