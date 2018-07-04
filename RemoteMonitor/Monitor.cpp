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
	ON_BN_CLICKED(IDC_START_RECORD, &CMonitor::OnBnClickedStartRecord)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_STOP_RECORD, &CMonitor::OnBnClickedStopRecord)
	ON_BN_CLICKED(IDC_PLAY, &CMonitor::OnBnClickedPlay)
END_MESSAGE_MAP()


// CMonitor 消息处理程序

/**
*	监视端与被监视端的SOCKET通信
*/
LRESULT CMonitor::OnSocket(WPARAM wParam, LPARAM lParam){
	switch(lParam){
		case FD_ACCEPT:{
			if(has_client || !can_accept){
				MessageBox("客户端无法连接");
				break;
			}
			SOCKADDR_IN addr;
			int len = sizeof(SOCKADDR_IN);
			socket_client = accept(socket_server,(sockaddr *)&addr,&len);
			if(INVALID_SOCKET == socket_client){
				MessageBox("客户端套接字创建出错");
				this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
				this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
				this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
				return -1;
			} else {
				//获取对方IP
				m_link = "与被监视端";
				m_link += inet_ntoa(addr.sin_addr);
				m_link += "连接成功";
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
				//接收文件大小并保存到file_size
				if(!recv(socket_client,(char*)&file_size,sizeof(unsigned long long)+1,NULL)){
					MessageBox("服务器接收文件大小时出错");
				}

				//文件大小大于0才读文件并且保存
				if(file_size>0){
					DWORD dwNumberOfBytesRecv = 0;		//接收到的字节数
					DWORD dwCountOfBytesRecv = 0;		//已接收文件大小
					char Buffer[512];
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

					//屏幕录制
					if(is_record){
						CString index;
						index.Format("%d",record_num);
						CString recordFilename = "Record\\Image"+index+".jpg";
						record_num++;
						if(!CopyFileA(filename,recordFilename,false)){
							MessageBox("录制失败！");
						}
						if(record_num>200){
							is_record = false;
							MessageBox("录制成功，请点击播放按钮");
							//WSACleanup();				//卸载winsock动态库
							closesocket(socket_client);
							has_client = false;
							can_accept = true;
							this->GetDlgItem(IDC_START_RECORD)->EnableWindow(true);
							this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
							this->GetDlgItem(IDC_PLAY)->EnableWindow(true);
						}
					}
					if(SOCKET_ERROR == send(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL)){
						send(socket_client,(char*)&buff,sizeof(unsigned char)+1,NULL);		//再次尝试发送
						//MessageBox("消息发送错误");
						return 1;
					}
				}
				is_recv = false;
			}
			break;
		}
		case FD_CLOSE :{
			MessageBox("被监视端已断开连接");
			has_client = false;
			can_accept = true;		//接受其他客户端连接
			this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
			this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
			this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
			m_link = "监视端正在等待被监视端连接... ...";
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
	//pw->ReleaseDC(pdc);		//释放PDC
	ReleaseDC(pdc);
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
	if( WSAAsyncSelect(socket_server,this->m_hWnd,WM_SOCKET,FD_ACCEPT | FD_READ | FD_CLOSE))
	{
		MessageBox("异步设置出错");
		return;
	}

	m_link = "监视端已启动，等待被监视端连接... ...";
	this->GetDlgItem(IDC_START)->EnableWindow(false);		//不可再次启动监视器
	UpdateData(false);
}


BOOL CMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	has_client = false;				//是否已有客户端连接
	is_recv = false;				//是否正在接收文件 如果正在接收图片  则不进行其他socket接收 防止读文件失败
	is_record = false;				//屏幕录制标志
	can_accept = true;				//是否接受其他客户端连接(回放的时候虽然没有客户端连接 但是不能让其他客户端连接上来  播放结束才可以)
	record_num = 0;					//当前录制了多少张图
	play_index = 0;					//当前播放到第几张图

	//获取窗口Picture区域大小
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

	//创建文件夹
	CString file = "Recv";
	if(!PathIsDirectory(file)){
		CreateDirectory(file,NULL);
	}
	CString record_file = "Record";
		if(!PathIsDirectory(record_file)){
		CreateDirectory(record_file,NULL);
	}

	//根据连接状态控制控件是否可点击
	this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_PLAY)->EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


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
	//全屏显示
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
		GetDlgItem(IDC_START_RECORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STOP_RECORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PLAY)->ShowWindow(SW_HIDE);

		//将PICTURE控件的坐标设为全屏大小  
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
		//显示控件
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
*	启动录制
*/
void CMonitor::OnBnClickedStartRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	if (has_client) {
		DeleteDirectory("Record");			//先清空文件夹
		CreateDirectory("Record",NULL);		//再次创建文件夹 可以让用户再次录屏
		record_num = 0;
		is_record = true;
		this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
		this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(true);
		this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
	} else {
		MessageBox("当前未连接被监视端，无法录屏");
	}
}


void CMonitor::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	play_index = record_num;			//让播放录屏的子线程自动退出  再执行删除文件操作
	DeleteDirectory("Record"); 
	CDialogEx::OnClose();
}

/**
*	删除文件夹及以下文件
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

	   if( (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) //发现目录
	   {
		if( !strcmp(FindData.cFileName, ".") || !strcmp(FindData.cFileName, "..") ) //.或..
		 continue;
		else
		{
		 char tmppath[MAX_PATH] = {0};
		 sprintf(tmppath, "%s\\%s", DirName, FindData.cFileName);
    
		 DeleteDirectory(tmppath);
		}
	   }
	   else               //发现文件
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
*	结束录屏
*/
void CMonitor::OnBnClickedStopRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	if (has_client) {
		is_record = false;
		//closesocket(socket_client);
		this->GetDlgItem(IDC_START_RECORD)->EnableWindow(true);
		this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
		this->GetDlgItem(IDC_PLAY)->EnableWindow(true);
	} else {
		MessageBox("未连接被监视端");
	}

}

/**
*	开始播放  断开socket
*/
void CMonitor::OnBnClickedPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	//使用线程睡眠方式播放
	closesocket(socket_client);
	m_link = "监视端已启动，等待被监视端连接... ...";
	UpdateData(false);
	//WSACleanup();				//卸载winsock动态库
	has_client = false;
	can_accept = false;
	this->GetDlgItem(IDC_START_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_STOP_RECORD)->EnableWindow(false);
	this->GetDlgItem(IDC_PLAY)->EnableWindow(false);
	//this->GetDlgItem(IDC_START)->EnableWindow(true);		//可重新启动监视器
	play_index = 0;
	HANDLE hThread = CreateThread(NULL,0,Play,this,0,NULL);
	//关闭该接收线程句柄，释放引用计数
	CloseHandle(hThread);
}

DWORD WINAPI CMonitor::Play(LPVOID lpParameter){
	CMonitor *pThis = (CMonitor*)lpParameter;
	pThis->pw = pThis->GetDlgItem(IDC_IMAGE);
	while(pThis->play_index<pThis->record_num){		//未播放结束则一直播放下一张图片
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
		//pw->ReleaseDC(pdc);		//释放PDC
		pThis->ReleaseDC(pThis->pdc);
		pThis->Img.Destroy();
		pThis->play_index++;
		Sleep(100);
	}
	pThis->GetDlgItem(IDC_PLAY)->EnableWindow(true);		//播放结束  可以按再次播放
	pThis->can_accept = true;								//可接受其他客户端连接
	return 0;
}


