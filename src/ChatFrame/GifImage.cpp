#include "StdAfx.h"
#include "ChatFrame/GifImage.h"

CGifImage::CGifImage(void)
{
	m_pImage = NULL;
	m_nFrameCnt = 0;
	m_nFramePos = 0;
	m_pFrameDelay = NULL;
}

CGifImage::~CGifImage(void)
{
	Destroy();
}

BOOL CGifImage::LoadFromFile(LPCTSTR pszFileName)
{
	Destroy();

	if (NULL == pszFileName || NULL == *pszFileName)
		return FALSE;
#if defined(UNICODE) || defined(_UNICODE)
	m_pImage = new Gdiplus::Image(pszFileName);
#else
	LPCWSTR pszFileNameW = AnsiToUnicode(pszFileName);
	if (pszFileNameW != NULL)
	{
		m_pImage = new Gdiplus::Image(pszFileNameW);
		delete pszFileNameW;
	}
#endif
	if (NULL == m_pImage)
		return FALSE;
	
	if (m_pImage->GetLastStatus() != Gdiplus::Ok)
	{
		delete m_pImage;
		m_pImage = NULL;
		return FALSE;
	}

	UINT nCount = 0;
	nCount = m_pImage->GetFrameDimensionsCount();
	if (nCount <= 0)
		return FALSE;

	GUID* pDimensionIDs = new GUID[nCount];
	if (pDimensionIDs != NULL)
	{
		m_pImage->GetFrameDimensionsList(pDimensionIDs, nCount);
		m_nFrameCnt = m_pImage->GetFrameCount(&pDimensionIDs[0]);
		delete pDimensionIDs;
	}

	if (m_nFrameCnt <= 1)
		return TRUE;

	UINT nSize = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay);
	if (nSize <= 0)
		return FALSE;

	Gdiplus::PropertyItem * pPropertyItem = (Gdiplus::PropertyItem *)malloc(nSize);
	if (pPropertyItem != NULL)
	{
		m_pImage->GetPropertyItem(PropertyTagFrameDelay, nSize, pPropertyItem);

		m_pFrameDelay = new long[m_nFrameCnt];
		if (m_pFrameDelay != NULL)
		{
			for (int i = 0; i < (int)m_nFrameCnt; i++)
			{
				m_pFrameDelay[i] = ((long*)pPropertyItem->value)[i] * 10;	// 帧切换延迟时间，以1/100秒为单位
				if (m_pFrameDelay[i] < 100)
					m_pFrameDelay[i] = 100;
			}
		}
		free(pPropertyItem);
	}

	return TRUE;
}

BOOL CGifImage::LoadFromIStream(IStream* pStream)
{
	return TRUE;
}

BOOL CGifImage::LoadFromBuffer(const BYTE* lpBuf, DWORD dwSize)
{
	return TRUE;
}

BOOL CGifImage::LoadFromResource(HINSTANCE hInstance, LPCTSTR pszResourceName, LPCTSTR pszResType)
{
	return TRUE;
}

BOOL CGifImage::LoadFromResource(HINSTANCE hInstance, UINT nIDResource, LPCTSTR pszResType)
{
	return TRUE;
}

BOOL CGifImage::SaveAsFile(LPCTSTR pszFileName)
{
	if (NULL == pszFileName || NULL == m_pImage)
		return FALSE;

	LPCTSTR lpExtension = _tcsrchr(pszFileName, _T('.'));
	if (NULL == lpExtension)
		return FALSE;

#if defined(UNICODE) || defined(_UNICODE)
	CLSID clsid = GetEncoderClsidByExtension(lpExtension);
#else
	CLSID	clsid = CLSID_NULL;
	LPCWSTR pszExtensionW = AnsiToUnicode(lpExtension);
	if (pszExtensionW != NULL)
	{
		clsid = GetEncoderClsidByExtension(pszExtensionW);
		delete pszExtensionW;
	}
#endif
	
	if (CLSID_NULL == clsid)
		return FALSE;

#if defined(UNICODE) || defined(_UNICODE)
	Gdiplus::Status status = m_pImage->Save(pszFileName, &clsid, NULL);
#else
	Gdiplus::Status status = Gdiplus::GenericError;
	LPCWSTR pszFileNameW = AnsiToUnicode(pszFileName);
	if (pszFileNameW != NULL)
	{
		status = m_pImage->Save(pszFileNameW, &clsid, NULL);
		delete pszFileNameW;
	}
#endif
	return (status != Gdiplus::Ok) ? FALSE : TRUE;
}

void CGifImage::Destroy()
{
	m_nFrameCnt = 0;
	m_nFramePos = 0;

	if (m_pFrameDelay != NULL)
	{
		delete []m_pFrameDelay;
		m_pFrameDelay = NULL;
	}

	if (m_pImage != NULL)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
}

UINT CGifImage::GetFrameCount()
{
	return m_nFrameCnt;
}

BOOL CGifImage::IsAnimatedGif()
{
	return m_nFrameCnt > 1;
}

long CGifImage::GetFrameDelay(int nFramePos/* = -1*/)
{
	if (!IsAnimatedGif() || NULL == m_pFrameDelay)
		return 0;

	int nFramePos2;
	if (nFramePos != -1)
		nFramePos2 = nFramePos;
	else
		nFramePos2 = m_nFramePos;

	if (nFramePos2 >= 0 && nFramePos2 < (int)m_nFrameCnt)
		return m_pFrameDelay[nFramePos2];
	else
		return 0;
}

void CGifImage::ActiveNextFrame()
{
	if (m_pImage != NULL && IsAnimatedGif())
	{
		m_nFramePos++;
		if (m_nFramePos == m_nFrameCnt)
			m_nFramePos = 0;

		if (m_nFramePos >= 0 && m_nFramePos < m_nFrameCnt)
		{
			static GUID Guid = Gdiplus::FrameDimensionTime;
			Gdiplus::Status status = m_pImage->SelectActiveFrame(&Guid, m_nFramePos);
		}
	}
}

void CGifImage::SelectActiveFrame(int nFramePos)
{
	if (m_pImage != NULL && IsAnimatedGif() 
		&& nFramePos >= 0 && nFramePos < (int)m_nFrameCnt)
	{
		static GUID Guid = Gdiplus::FrameDimensionTime;
		Gdiplus::Status status = m_pImage->SelectActiveFrame(&Guid, nFramePos);
		m_nFramePos = nFramePos;
	}
}

BOOL CGifImage::Draw(HDC hDestDC, int xDest, int yDest, int nFramePos/* = -1*/)
{
// 	HDC hMemDC;
// 	HBITMAP hBitmap = NULL, hOldBmp;
// 	Gdiplus::Bitmap* pBitmap;
// 	int nWidth, nHeight;
// 
// 	if (NULL == m_pImage)
// 		return FALSE;
// 
// 	if (nFramePos != -1)
// 		SelectActiveFrame(nFramePos);
// 
// 	nWidth = m_pImage->GetWidth();
// 	nHeight = m_pImage->GetHeight();
// 
// 	pBitmap = (Gdiplus::Bitmap *)m_pImage;
// 	pBitmap->GetHBITMAP(Gdiplus::Color::Transparent, &hBitmap);
// 
// 	hMemDC = ::CreateCompatibleDC(hDestDC);
// 	hOldBmp = (HBITMAP)::SelectObject(hMemDC, hBitmap);
// 
// 	BLENDFUNCTION stBlendFunction = {0};
// 	stBlendFunction.BlendOp= AC_SRC_OVER;
// 	stBlendFunction.SourceConstantAlpha = 255;
// 	stBlendFunction.AlphaFormat = AC_SRC_ALPHA;
// 
// 	BOOL bStat = ::AlphaBlend(hDestDC, xDest, yDest, nWidth, nHeight,
// 		hMemDC, 0, 0, nWidth, nHeight, stBlendFunction);
// 
// 	::SelectObject(hMemDC, hOldBmp);
// 	::DeleteObject(hBitmap);
// 	::DeleteDC(hMemDC);
// 
// 	return TRUE;
 	Gdiplus::Graphics graphics(hDestDC);
 	Gdiplus::Status status = graphics.DrawImage(m_pImage, xDest, yDest);
 	if(status != Gdiplus::Ok)
 		return FALSE;
 	else
 		return TRUE;
}

BOOL CGifImage::Draw(HDC hDestDC, const RECT& rectDest, int nFramePos/* = -1*/)
{
// 	HDC hMemDC;
// 	HBITMAP hBitmap = NULL, hOldBmp;
// 	Gdiplus::Bitmap* pBitmap;
// 	int nWidth, nHeight;
// 
// 	if (NULL == m_pImage)
// 		return FALSE;
// 
// 	if (nFramePos != -1)
// 		SelectActiveFrame(nFramePos);
// 
// 	nWidth = m_pImage->GetWidth();
// 	nHeight = m_pImage->GetHeight();
//
// 	pBitmap = (Gdiplus::Bitmap *)m_pImage;
// 	pBitmap->GetHBITMAP(Gdiplus::Color::Transparent, &hBitmap);
// 
// 	hMemDC = ::CreateCompatibleDC(hDestDC);
// 	hOldBmp = (HBITMAP)::SelectObject(hMemDC, hBitmap);
// 
// 	BLENDFUNCTION stBlendFunction = {0};
// 	stBlendFunction.BlendOp= AC_SRC_OVER;
// 	stBlendFunction.SourceConstantAlpha = 255;
// 	stBlendFunction.AlphaFormat = AC_SRC_ALPHA;
// 
// 	BOOL bStat = ::AlphaBlend(hDestDC, rectDest.left, rectDest.top, 
// 		rectDest.right - rectDest.left, rectDest.bottom - rectDest.top,
// 		hMemDC, 0, 0, nWidth, nHeight, stBlendFunction);
// 
// 	::SelectObject(hMemDC, hOldBmp);
// 	::DeleteObject(hBitmap);
// 	::DeleteDC(hMemDC);
//
//	return TRUE;

	if (NULL == m_pImage)
		return FALSE;

	if (nFramePos != -1)
		SelectActiveFrame(nFramePos);

	int nWidth = rectDest.right-rectDest.left;
	int nHeight = rectDest.bottom-rectDest.top;

 	Gdiplus::Graphics graphics(hDestDC);
 	Gdiplus::Status status = graphics.DrawImage(m_pImage, 
 		Gdiplus::Rect(rectDest.left, rectDest.top, nWidth, nHeight));
 	if(status != Gdiplus::Ok)
 		return FALSE;
 	else
 		return TRUE;
}

int CGifImage::GetWidth()
{
	if (m_pImage != NULL)
		return m_pImage->GetWidth();
	else
		return 0;
}

int CGifImage::GetHeight()
{
	if (m_pImage != NULL)
		return m_pImage->GetHeight();
	else
		return 0;
}

UINT CGifImage::GetCurFramePos()
{
	return m_nFramePos;
}

BOOL CGifImage::GetRawFormat(GUID * lpGuid)
{
	Gdiplus::Status status = m_pImage->GetRawFormat(lpGuid);
	return (Gdiplus::Ok == status) ? TRUE : FALSE;
}

CLSID CGifImage::GetEncoderClsidByExtension(const WCHAR * lpExtension)
{
	CLSID clsid = CLSID_NULL;

	if (NULL == lpExtension)
		return clsid;

	UINT numEncoders = 0, size = 0;
	Gdiplus::Status status = Gdiplus::GetImageEncodersSize(&numEncoders, &size);  
	if (status != Gdiplus::Ok)
		return clsid;

	Gdiplus::ImageCodecInfo* lpEncoders = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (NULL == lpEncoders)
		return clsid;

	status = Gdiplus::GetImageEncoders(numEncoders, size, lpEncoders);
	if (Gdiplus::Ok == status)
	{
		for (UINT i = 0; i < numEncoders; i++)
		{
			BOOL bFind = FALSE;
			const WCHAR * pStart = lpEncoders[i].FilenameExtension;
			const WCHAR * pEnd = wcschr(pStart, L';');
			do 
			{
				if (NULL == pEnd)
				{
					LPCWSTR lpExt = ::wcsrchr(pStart, L'.');
					if ((lpExt != NULL) && (wcsicmp(lpExt, lpExtension) == 0))
					{
						clsid = lpEncoders[i].Clsid;
						bFind = TRUE;
					}
					break;
				}

				int nLen = pEnd-pStart;
				if (nLen < MAX_PATH)
				{
					WCHAR cBuf[MAX_PATH] = {0};
					wcsncpy(cBuf, pStart, nLen);
					LPCWSTR lpExt = ::wcsrchr(cBuf, L'.');
					if ((lpExt != NULL) && (wcsicmp(lpExt, lpExtension) == 0))
					{
						clsid = lpEncoders[i].Clsid;
						bFind = TRUE;
						break;
					}
				}
				pStart = pEnd+1;
				if (L'\0' == *pStart)
					break;
				pEnd = wcschr(pStart, L';');
			} while (1);
			if (bFind)
				break;
		}
	}

	free(lpEncoders);

	return clsid;
}