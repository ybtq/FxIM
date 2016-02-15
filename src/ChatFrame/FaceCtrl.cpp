#include "StdAfx.h"
#include "ChatFrame/FaceCtrl.h"

namespace DuiLib {

#define		TIMER_ID	1000

CFaceCtrl::CFaceCtrl(void)
{
	m_dwBackColor = RGB(255, 255, 255);
	m_clrLine = RGB(223, 230, 246);
	m_clrFocusBorder = RGB(0, 0, 255);
	m_clrZoomBorder = RGB(0, 138, 255);
	m_nRow = 8;
	m_nCol = 15;
	m_nItemWidth = m_nItemHeight = 28;
	m_nZoomWidth = m_nZoomHeight = 84;
	m_nCurPage = m_nPageCnt = 0;
	m_nHoverIndex = -1;
	m_nFramePos = 0;
	::SetRectEmpty(&m_rcZoomImg);
	m_lpFaceList = NULL;
}

CFaceCtrl::~CFaceCtrl(void)
{
	m_pManager->KillTimer(this, TIMER_ID);
	
	DestroyImage();

	m_nHoverIndex = -1;
	m_nFramePos = 0;
}

LPCTSTR CFaceCtrl::GetClass() const
{
	return _T("FaceCtrl");
}

LPVOID CFaceCtrl::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("FaceCtrl")) == 0 ) return static_cast<CFaceCtrl*>(this);
	return CControlUI::GetInterface(pstrName);
}

void CFaceCtrl::SetLineColor(COLORREF color)
{
	m_clrLine = color;
}

void CFaceCtrl::SetFocusBorderColor(COLORREF color)
{
	m_clrFocusBorder = color;
}

void CFaceCtrl::SetZoomBorderColor(COLORREF color)
{
	m_clrZoomBorder = color;
}

void CFaceCtrl::SetRowAndCol(int nRow, int nCol)
{
	m_nRow = nRow;
	m_nCol = nCol;
}

void CFaceCtrl::SetItemSize(int nWidth, int nHeight)
{
	m_nItemWidth = nWidth;
	m_nItemHeight = nHeight;
}

void CFaceCtrl::SetZoomSize(int nWidth, int nHeight)
{
	m_nZoomWidth = nWidth;
	m_nZoomHeight = nHeight;
}

void CFaceCtrl::SetFaceList(CFaceList * lpFaceList)
{
	m_lpFaceList = lpFaceList;
}

void CFaceCtrl::SetCurPage(int nPageIndex)
{
	if (NULL == m_lpFaceList)
		return;

	int nCount = (int)m_lpFaceList->m_arrFaceInfo.size();
	int nOnePage = m_nRow * m_nCol;
	if (nCount > 0 && nOnePage > 0)
	{
		if (nCount % nOnePage == 0)
			m_nPageCnt = nCount / nOnePage;
		else
			m_nPageCnt = nCount / nOnePage + 1;
	}
	else
		m_nPageCnt = 0;

	if (nPageIndex < 0 || nPageIndex >= m_nPageCnt)
		return;

	m_nCurPage = nPageIndex;
	LoadImage(m_nCurPage);
}

CFaceInfo * CFaceCtrl::GetFaceInfo(int nItemIndex)
{
	if (m_lpFaceList != NULL)
	{
		int nCount = (int)m_lpFaceList->m_arrFaceInfo.size();
		int nOnePage = m_nRow * m_nCol;
		int nStart = m_nCurPage * nOnePage;
		int nIndex = nStart + m_nHoverIndex;
		if (nIndex >= 0 && nIndex < nCount)
			return m_lpFaceList->m_arrFaceInfo[nIndex];
	}
	return NULL;
}

// SIZE CFaceCtrl::EstimateSize(SIZE szAvailable)
// {
// 	if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 4);
// 	return CControlUI::EstimateSize(szAvailable);
// }

void CFaceCtrl::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_SETFOCUS ) 
	{
		m_bFocused = true;
		return;
	}
	if( event.Type == UIEVENT_KILLFOCUS ) 
	{
		m_bFocused = false;
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		m_nHoverIndex = -1;
		this->Invalidate();
		return;
	}

	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		int nIndex = HitTest(event.ptMouse);
		if (nIndex != m_nHoverIndex)
		{
			SetToolTip(_T(""));
			::SendMessage(m_pManager->GetTooltipWindow(), TTM_TRACKACTIVATE, FALSE, 0L);
			m_pManager->KillTimer(this, TIMER_ID);
			m_nFramePos = 0;
			m_nHoverIndex = nIndex;
			if (m_nHoverIndex != -1)
			{
				::SendMessage(m_pManager->GetTooltipWindow(), TTM_ACTIVATE, TRUE, 0L);
				CFaceInfo * lpFaceInfo = GetFaceInfo(m_nHoverIndex);
				if (lpFaceInfo != NULL)
					SetToolTip(lpFaceInfo->m_strTip.c_str());

				CalcZoomRect(event.ptMouse);
				CGifImage * lpImage = GetZoomImage();
				if (lpImage != NULL)
				{
					if (lpImage->IsAnimatedGif())
						m_pManager->SetTimer(this, TIMER_ID, lpImage->GetFrameDelay(m_nFramePos));
				}
			}
			this->Invalidate();
		}
		return;
	}

	if( event.Type == UIEVENT_BUTTONDOWN )
	{
		int nIndex = HitTest(event.ptMouse);
		if (nIndex != -1)
		{
			CFaceInfo * lpFaceInfo = GetFaceInfo(nIndex);
			if (lpFaceInfo != NULL && m_pManager != NULL)
				m_pManager->SendNotify(this, _T("click"), 0, nIndex);
		}
		return;
	}

	if( event.Type == UIEVENT_TIMER )
	{
		if (event.wParam == TIMER_ID)
		{
			m_pManager->KillTimer(this, TIMER_ID);

			CGifImage * lpImage = GetZoomImage();
			if (lpImage != NULL)
			{
				m_nFramePos++;
				if (m_nFramePos >= (int)lpImage->GetFrameCount())
					m_nFramePos = 0;
				m_pManager->SetTimer(this, TIMER_ID, lpImage->GetFrameDelay(m_nFramePos));
			}
			this->Invalidate();
		}
		return;
	}
	
	CControlUI::DoEvent(event);
}

void CFaceCtrl::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	CControlUI::SetAttribute(pstrName, pstrValue);
}

void CFaceCtrl::PaintBkColor(HDC hDC)
{
	CControlUI::PaintBkColor(hDC);
	DrawLine(hDC);
}

void CFaceCtrl::PaintBkImage(HDC hDC)
{
	CControlUI::PaintBkImage(hDC);
}

void CFaceCtrl::PaintStatusImage(HDC hDC)
{
	RECT rcItem, rcImage;
	int nRow = 0, nCol = 0;
	int nLeft = m_rcItem.left, nTop = m_rcItem.top;
	for (int i = 0; i < (int)m_arrImage.size(); i++)
	{
		::SetRect(&rcItem, nLeft, nTop, nLeft+m_nItemWidth, nTop+m_nItemHeight);

		CGifImage * lpImage = m_arrImage[i];
		if (lpImage != NULL)
		{
			CalcCenterRect(rcItem, lpImage->GetWidth(), lpImage->GetHeight(), rcImage);
			lpImage->Draw(hDC, rcImage, 0);
		}

		if (m_nHoverIndex == i)
			DrawFocusBorder(hDC, rcItem);

		nLeft += m_nItemWidth;
		nCol++;
		if (nCol >= m_nCol)
		{
			nCol = 0;
			nRow++;
			if (nRow > m_nRow)
				break;
			nLeft = m_rcItem.left;
			nTop += m_nItemHeight;
		}
	}

	if (m_nHoverIndex != -1)
		DrawZoomImage(hDC);
}

void CFaceCtrl::CalcCenterRect(RECT& rcDest, int cx, int cy, RECT& rcCenter)
{
	int nWidth = rcDest.right-rcDest.left;
	int nHeight = rcDest.bottom-rcDest.top;

	if (cx > nWidth)
		cx = nWidth - 2;

	if (cy > nHeight)
		cy = nHeight - 2;

	int x = (nWidth - cx + 1) / 2;
	int y = (nHeight - cy + 1) / 2;

	::SetRect(&rcCenter, rcDest.left+x, rcDest.top+y, rcDest.left+x+cx, rcDest.top+y+cy);
}

int CFaceCtrl::HitTest(POINT pt)
{
	RECT rcItem;
	int nRow = 0, nCol = 0;
	int nLeft = m_rcItem.left, nTop = m_rcItem.top;
	for (int i = 0; i < (int)m_arrImage.size(); i++)
	{
		::SetRect(&rcItem, nLeft, nTop, nLeft+m_nItemWidth, nTop+m_nItemHeight);
		if (::PtInRect(&rcItem, pt))
			return i;

		nLeft += m_nItemWidth;
		nCol++;
		if (nCol >= m_nCol)
		{
			nCol = 0;
			nRow++;
			if (nRow > m_nRow)
				break;
			nLeft = m_rcItem.left;
			nTop += m_nItemHeight;
		}
	}

	return -1;
}

void CFaceCtrl::CalcZoomRect(POINT point)
{
	int nWidth = m_rcItem.right-m_rcItem.left;
	int nCenter = nWidth / 2;
	if (point.x < nCenter)
	{
		m_rcZoomImg.left = m_rcItem.right - m_nZoomWidth;
		m_rcZoomImg.right = m_rcItem.right;
		m_rcZoomImg.top = m_rcItem.top + 1;
		m_rcZoomImg.bottom = m_rcZoomImg.top + m_nZoomHeight;
	}
	else
	{
		m_rcZoomImg.left = m_rcItem.left + 1;
		m_rcZoomImg.right = m_rcZoomImg.left + m_nZoomWidth;
		m_rcZoomImg.top = m_rcItem.top + 1;
		m_rcZoomImg.bottom = m_rcZoomImg.top + m_nZoomHeight;
	}
}

BOOL CFaceCtrl::GetItemRect(int nItemIndex, RECT& rect)
{
	::SetRectEmpty(&rect);

	if (nItemIndex < 0 || nItemIndex >= (int)m_arrImage.size())
		return FALSE;

	RECT rcItem;
	int nRow = 0, nCol = 0;
	int nLeft = m_rcItem.left, nTop = m_rcItem.top;
	for (int i = 0; i < (int)m_arrImage.size(); i++)
	{
		::SetRect(&rcItem, nLeft, nTop, nLeft+m_nItemWidth, nTop+m_nItemHeight);
		if (i == nItemIndex)
		{
			rect = rcItem;
			return TRUE;
		}

		nLeft += m_nItemWidth;
		nCol++;
		if (nCol >= m_nCol)
		{
			nCol = 0;
			nRow++;
			if (nRow > m_nRow)
				break;
			nLeft = m_rcItem.left;
			nTop += m_nItemHeight;
		}
	}

	return FALSE;
}

CGifImage * CFaceCtrl::GetZoomImage()
{
	if (m_nHoverIndex >= 0 && m_nHoverIndex < (int)m_arrImage.size())
		return m_arrImage[m_nHoverIndex];
	else
		return NULL;
}

void CFaceCtrl::DrawLine(HDC hDC)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 1, m_clrLine);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);

	int x1 = m_rcItem.left, y1 = m_rcItem.top, x2 = m_rcItem.right, y2 = m_rcItem.top;
	for (int i = 0; i < m_nRow; i++)
	{
		::MoveToEx(hDC, x1, y1, NULL);
		::LineTo(hDC, x2, y2);
		y1 += m_nItemHeight;
		y2 = y1;
	}

	x1 = m_rcItem.left, y1 = m_rcItem.top, x2 = m_rcItem.left, y2 = m_rcItem.bottom;
	for (int i = 0; i < m_nCol; i++)
	{
		::MoveToEx(hDC, x1, y1, NULL);
		::LineTo(hDC, x2, y2);
		x1 += m_nItemWidth;
		x2 = x1;
	}

	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
}

void CFaceCtrl::DrawFocusBorder(HDC hDC, const RECT& rect)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 1, m_clrFocusBorder);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)::GetStockObject(NULL_BRUSH));
	::Rectangle(hDC, rect.left+1, rect.top+1, rect.right, rect.bottom);
	::SelectObject(hDC, hOldBrush);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
}

void CFaceCtrl::DrawZoomImage(HDC hDC)
{
	COLORREF clrBg = m_dwBackColor;
	if (clrBg > 0xFF000000)
		clrBg -= 0xFF000000;

	HPEN hPen = ::CreatePen(PS_SOLID, 1, m_clrZoomBorder);
	HBRUSH hBrush = ::CreateSolidBrush(clrBg);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, hBrush);
	::Rectangle(hDC, m_rcZoomImg.left, m_rcZoomImg.top, m_rcZoomImg.right, m_rcZoomImg.bottom);
	::SelectObject(hDC, hOldBrush);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hBrush);
	::DeleteObject(hPen);

	if (m_nHoverIndex >= 0 && m_nHoverIndex < (int)m_arrImage.size())
	{
		CGifImage * lpImage = m_arrImage[m_nHoverIndex];
		if (lpImage != NULL)
		{
			RECT rcZoomImg;
			CalcCenterRect(m_rcZoomImg, lpImage->GetWidth(), 
				lpImage->GetHeight(), rcZoomImg);
			lpImage->Draw(hDC, rcZoomImg, m_nFramePos);
		}
	}
}

BOOL CFaceCtrl::LoadImage(int nPageIndex)
{
	if (NULL == m_lpFaceList)
		return FALSE;

	DestroyImage();

	int nCount = (int)m_lpFaceList->m_arrFaceInfo.size();
	int nOnePage = m_nRow * m_nCol;

	int nStart = nPageIndex * nOnePage;
	int nEnd = nStart + nOnePage;
	if (nEnd > nCount)
		nEnd = nCount;

	for (int i = nStart; i < nEnd; i++)
	{
		CFaceInfo * lpFaceInfo = m_lpFaceList->m_arrFaceInfo[i];
		if (lpFaceInfo != NULL)
		{
			CGifImage * lpImage = new CGifImage;
			if (lpImage != NULL)
			{
				BOOL bRet = lpImage->LoadFromFile(lpFaceInfo->m_strFileName.c_str());
				if (bRet)
					m_arrImage.push_back(lpImage);
				else
					delete lpImage;
			}
		}
	}

	return TRUE;
}

void CFaceCtrl::DestroyImage()
{
	for (int i = 0; i < (int)m_arrImage.size(); i++)
	{
		CGifImage * lpImage = m_arrImage[i];
		if (lpImage != NULL)
			delete lpImage;
	}
	m_arrImage.clear();
}

} // namespace DuiLib