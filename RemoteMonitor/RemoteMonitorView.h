
// RemoteMonitorView.h : CRemoteMonitorView 类的接口
//

#pragma once


class CRemoteMonitorView : public CView
{
protected: // 仅从序列化创建
	CRemoteMonitorView();
	DECLARE_DYNCREATE(CRemoteMonitorView)

// 特性
public:
	CRemoteMonitorDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CRemoteMonitorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // RemoteMonitorView.cpp 中的调试版本
inline CRemoteMonitorDoc* CRemoteMonitorView::GetDocument() const
   { return reinterpret_cast<CRemoteMonitorDoc*>(m_pDocument); }
#endif

