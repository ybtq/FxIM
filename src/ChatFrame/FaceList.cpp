#include "StdAfx.h"
#include "ChatFrame/FaceList.h"
#include "ThirdParty/tinyxml/tinyxml.h"

CFaceInfo::CFaceInfo(void)
{
	m_nId = -1;
	m_nIndex = -1;
	m_strTip = _T("");
	m_strFileName = _T("");
}

CFaceInfo::~CFaceInfo(void)
{

}

CFaceList::CFaceList(void)
{
	Reset();
}

CFaceList::~CFaceList(void)
{
	Reset();
}

void CFaceList::Reset()
{
// 	m_nWidth = 28;
// 	m_nHeight = 28;
// 	m_nZoomWidth = 86;
// 	m_nZoomHeight = 86;
// 	m_nCol = 15;
	m_nItemWidth = m_nItemHeight = 0;
	m_nZoomWidth = m_nZoomHeight = 0;
	m_nRow = m_nCol = 0;
	
	for (int i = 0; i < (int)m_arrFaceInfo.size(); i++)
	{
		CFaceInfo * lpFaceInfo = m_arrFaceInfo[i];
		if (lpFaceInfo != NULL)
			delete lpFaceInfo;
	}
	m_arrFaceInfo.clear();
}

BOOL CFaceList::LoadConfigFile(LPCTSTR lpszFileName)
{
	TiXmlDocument	xmlDoc;
	TiXmlNode*		xmlNode;

	if (!xmlDoc.LoadFile(lpszFileName)) {
		return FALSE;
	}

	Reset();

	TiXmlElement* rootElement = xmlDoc.RootElement();
	if ((xmlNode = xmlDoc.FirstChild("faceconfig")) != NULL)
	{
		TiXmlElement*	xmlElement = xmlNode->ToElement();

		xmlElement->Attribute(_T("item_width"), &m_nItemWidth);
		xmlElement->Attribute(_T("item_height"), &m_nItemHeight);
		xmlElement->Attribute(_T("zoom_width"), &m_nZoomWidth);
		xmlElement->Attribute(_T("zoom_height"), &m_nZoomHeight);
		xmlElement->Attribute(_T("row"), &m_nRow);
		xmlElement->Attribute(_T("col"), &m_nCol);

		TiXmlElement*	xmlSubElement;
		xmlSubElement = xmlNode->FirstChildElement(_T("face"));
		while (xmlSubElement != NULL) 
		{
			CFaceInfo * lpFaceInfo = new CFaceInfo;
			if (lpFaceInfo != NULL) 
			{
				xmlSubElement->Attribute(_T("id"), &lpFaceInfo->m_nId);
#if (defined(UNICODE) || defined(_UNICODE))
				std::string strTip = xmlSubElement->Attribute(_T("tip"));
				std::string strFileName = xmlSubElement->Attribute(_T("file"));
				if (!strTip.empty()) {
					WCHAR*	szTip = Utf8ToUnicode(strTip.data());
					if (szTip != NULL) {
						lpFaceInfo->m_strTip = szTip;
						delete szTip;
					}					
				}
				if (!strFileName.empty())
				{
					WCHAR*	szFileName = Utf8ToUnicode(strFileName.data());
					if (szFileName != NULL) {
						lpFaceInfo->m_strFileName = szFileName;
						delete szFileName;
					}					
				}
#else
				lpFaceInfo->m_strTip = xmlSubElement->Attribute(_T("tip"));
				lpFaceInfo->m_strFileName = xmlSubElement->Attribute(_T("file"));
				ConvertUtf8ToGBK(lpFaceInfo->m_strTip);
				ConvertUtf8ToGBK(lpFaceInfo->m_strFileName);
#endif
				lpFaceInfo->m_strFileName = CPaintManagerUI::GetInstancePath() + lpFaceInfo->m_strFileName.data();
				tstring strIndex = GetFileNameWithoutExtension(lpFaceInfo->m_strFileName.data());
				if (IsDigit(strIndex.c_str())) {
					lpFaceInfo->m_nIndex = _tcstol(strIndex.c_str(), NULL, 10);
				}
				m_arrFaceInfo.push_back(lpFaceInfo);
			}		

			xmlSubElement = xmlSubElement->NextSiblingElement(_T("face"));
		}
	}

	return TRUE;
}

CFaceInfo * CFaceList::GetFaceInfo(int nIndex)
{
	if (nIndex >= 0 && nIndex < (int)m_arrFaceInfo[nIndex])
		return m_arrFaceInfo[nIndex];
	else
		return NULL;
}

CFaceInfo * CFaceList::GetFaceInfoById(int nFaceId)
{
	for (int i = 0; i < (int)m_arrFaceInfo.size(); i++)
	{
		CFaceInfo * lpFaceInfo = m_arrFaceInfo[i];
		if (lpFaceInfo != NULL && lpFaceInfo->m_nId == nFaceId)
			return lpFaceInfo;
	}

	return NULL;
}

CFaceInfo * CFaceList::GetFaceInfoByIndex(int nFaceIndex)
{
	for (int i = 0; i < (int)m_arrFaceInfo.size(); i++)
	{
		CFaceInfo * lpFaceInfo = m_arrFaceInfo[i];
		if (lpFaceInfo != NULL && lpFaceInfo->m_nIndex == nFaceIndex)
			return lpFaceInfo;
	}

	return NULL;
}