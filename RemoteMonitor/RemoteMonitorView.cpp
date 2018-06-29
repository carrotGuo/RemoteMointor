
// RemoteMonitorView.cpp : CRemoteMonitorView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "RemoteMonitor.h"
#endif

#include "RemoteMonitorDoc.h"
#include "RemoteMonitorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRemoteMonitorView

IMPLEMENT_DYNCREATE(CRemoteMonitorView, CView)

BEGIN_MESSAGE_MAP(CRemoteMonitorView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CRemoteMonitorView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CRemoteMonitorView ����/����

CRemoteMonitorView::CRemoteMonitorView()
{
	// TODO: �ڴ˴���ӹ������

}

CRemoteMonitorView::~CRemoteMonitorView()
{
}

BOOL CRemoteMonitorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CRemoteMonitorView ����

void CRemoteMonitorView::OnDraw(CDC* /*pDC*/)
{
	CRemoteMonitorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CRemoteMonitorView ��ӡ


void CRemoteMonitorView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CRemoteMonitorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CRemoteMonitorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CRemoteMonitorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
}

void CRemoteMonitorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CRemoteMonitorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CRemoteMonitorView ���

#ifdef _DEBUG
void CRemoteMonitorView::AssertValid() const
{
	CView::AssertValid();
}

void CRemoteMonitorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRemoteMonitorDoc* CRemoteMonitorView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRemoteMonitorDoc)));
	return (CRemoteMonitorDoc*)m_pDocument;
}
#endif //_DEBUG


// CRemoteMonitorView ��Ϣ�������
