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
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDBLCLK()
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
				bool can_send = true;
				byte buff = (unsigned char)can_send;
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
					if(SOCKET_ERROR == send(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL)){
						MessageBox("消息发送错误");
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
	
	//窗口全屏 但是控件没有变化
	//int cxScreen,cyScreen; 
	//cxScreen=GetSystemMetrics(SM_CXSCREEN);
	//cyScreen=GetSystemMetrics(SM_CYSCREEN);
	//SetWindowPos(&wndTopMost,0,0,cxScreen,cyScreen,SWP_SHOWWINDOW);

	has_client = false;
	is_recv = false;

	//获取窗口Picture区域大小
	pw = this->GetDlgItem(IDC_IMAGE);
	CRect rect;
	pw->GetClientRect(&rect);
	ww = rect.Width();
	wh = rect.Height();

	bFullScreen = false;
	bmp1.LoadBitmapA(IDB_BITMAP2);
	bmp2.LoadBitmapA(IDB_BITMAP3);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//BOOL CMonitor::PreTranslateMessage(MSG* pMsg)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//	if (WM_LBUTTONDBLCLK == pMsg->message) {
//		if (!bFullScreen) {  
//			bFullScreen = true;  
//			//获取系统屏幕宽高  
//			int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);  
//			int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);  
//  
//			//用m_struOldWndpl得到当前窗口的显示状态和窗体位置，以供退出全屏后使用  
//			GetWindowPlacement(&m_struOldWndpl);  
//			GetDlgItem(IDC_IMAGE)->GetWindowPlacement(&m_struOldWndpPic);  
//      
//			//计算出窗口全屏显示客户端所应该设置的窗口大小，主要为了将不需要显示的窗体边框等部分排除在屏幕外  
//			CRect rectWholeDlg;  
//			CRect rectClient;  
//			GetWindowRect(&rectWholeDlg);//得到当前窗体的总的相对于屏幕的坐标  
//			RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);//得到客户区窗口坐标  
//			ClientToScreen(&rectClient);//将客户区相对窗体的坐标转为相对屏幕坐标  
//			//GetDlgItem(IDC_STATIC_PICSHOW)->GetWindowRect(rectClient);//得到PICTURE控件坐标  
//  
//			rectFullScreen.left = rectWholeDlg.left - rectClient.left;  
//			rectFullScreen.top = rectWholeDlg.top - rectClient.top;  
//			rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;  
//			rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;  
//  
//			//设置窗口对象参数，为全屏做好准备并进入全屏状态  
//			WINDOWPLACEMENT struWndpl;  
//			struWndpl.length = sizeof(WINDOWPLACEMENT);   
//			struWndpl.flags = 0;  
//			struWndpl.showCmd = SW_SHOWNORMAL;  
//			struWndpl.rcNormalPosition = rectFullScreen;  
//			SetWindowPlacement(&struWndpl);//该函数设置指定窗口的显示状态和显示大小位置等，是我们该程序最为重要的函数  
//  
//			//将PICTURE控件的坐标设为全屏大小  
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!bFullScreen)  
	{  
		bFullScreen = true;  
  
		//获取系统屏幕宽高  
		int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);  
		int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);  
  
		//用m_struOldWndpl得到当前窗口的显示状态和窗体位置，以供退出全屏后使用  
		GetWindowPlacement(&m_struOldWndpl);  
		GetDlgItem(IDC_IMAGE)->GetWindowPlacement(&m_struOldWndpPic);  
      
		//计算出窗口全屏显示客户端所应该设置的窗口大小，主要为了将不需要显示的窗体边框等部分排除在屏幕外  
		CRect rectWholeDlg;  
		CRect rectClient;  
		GetWindowRect(&rectWholeDlg);//得到当前窗体的总的相对于屏幕的坐标  
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);//得到客户区窗口坐标  
		ClientToScreen(&rectClient);//将客户区相对窗体的坐标转为相对屏幕坐标  
		//GetDlgItem(IDC_IMAGE)->GetWindowRect(rectClient);//得到PICTURE控件坐标  
  
		rectFullScreen.left = rectWholeDlg.left - rectClient.left;  
		rectFullScreen.top = rectWholeDlg.top - rectClient.top;  
		rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;  
		rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;  
  
		//设置窗口对象参数，为全屏做好准备并进入全屏状态  
		WINDOWPLACEMENT struWndpl;  
		struWndpl.length = sizeof(WINDOWPLACEMENT);   
		struWndpl.flags = 0;  
		struWndpl.showCmd = SW_SHOWNORMAL;  
		struWndpl.rcNormalPosition = rectFullScreen;  
		SetWindowPlacement(&struWndpl);//该函数设置指定窗口的显示状态和显示大小位置等，是我们该程序最为重要的函数  
  
		//隐藏控件
        GetDlgItem(IDC_STATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_START)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);

		//将PICTURE控件的坐标设为全屏大小  
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
		//显示控件
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
