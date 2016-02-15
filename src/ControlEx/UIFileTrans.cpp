#include "StdAfx.h"
#include <math.h>
#include "Utils/BaseDef.h"
#include "MainFrame/ShareMng.h"
#include "ControlEx/UIFileTrans.h"

namespace DuiLib
{

const TCHAR kSendFileElementUIClassName[]			= _T("SendFileElementUI");
const TCHAR kSendFileElementUIInterfaceName[]		= _T("SendFileElement");
const TCHAR kRecvFileElementUIClassName[]			= _T("RecvFileElementUI");
const TCHAR kRecvFileElementUIInterfaceName[]		= _T("RecvFileElement");

CFileTransUI::CFileTransUI()
: m_lDelayDeltaY(0)
, m_lDelayNumber(0)
, m_lDelayLeft(0)
, m_nShareCount(0)
{

}

CFileTransUI::~CFileTransUI()
{

}

double CFileTransUI::CalculateDelay(double state)
{
	return pow(state, 2);
}

CControlUI*	CFileTransUI::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, kSendFileElementUIInterfaceName) == 0)
	{
		return new CSendFileElementUI();
	}
	else if (_tcsicmp(pstrClass, kRecvFileElementUIInterfaceName) == 0)
	{
		return new CRecvFileElementUI();
	}
	return NULL;
}

void CFileTransUI::DoEvent(TEventUI& event)
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
	{
		if (m_pParent != NULL)
			m_pParent->DoEvent(event);
		else
			CVerticalLayoutUI::DoEvent(event);
		return;
	}

	if (event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID)
	{
		if (m_lDelayLeft > 0)
		{
			--m_lDelayLeft;
			SIZE sz = GetScrollPos();
			LONG lDeltaY =  (LONG)(CalculateDelay((double)m_lDelayLeft / m_lDelayNumber) * m_lDelayDeltaY);
			if ((lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ))
			{
				sz.cy -= lDeltaY;
				SetScrollPos(sz);
				return;
			}
		}
		m_lDelayDeltaY = 0;
		m_lDelayNumber = 0;
		m_lDelayLeft = 0;
		m_pManager->KillTimer(this, SCROLL_TIMERID);
		return;
	}
	if (event.Type == UIEVENT_SCROLLWHEEL)
	{
		LONG lDeltaY = 0;
		if (m_lDelayNumber > 0)
			lDeltaY =  (LONG)(CalculateDelay((double)m_lDelayLeft / m_lDelayNumber) * m_lDelayDeltaY);
		switch (LOWORD(event.wParam))
		{
		case SB_LINEUP:
			if (m_lDelayDeltaY >= 0)
				m_lDelayDeltaY = lDeltaY + 8;
			else
				m_lDelayDeltaY = lDeltaY + 12;
			break;
		case SB_LINEDOWN:
			if (m_lDelayDeltaY <= 0)
				m_lDelayDeltaY = lDeltaY - 8;
			else
				m_lDelayDeltaY = lDeltaY - 12;
			break;
		}
		if
			(m_lDelayDeltaY > 100) m_lDelayDeltaY = 100;
		else if
			(m_lDelayDeltaY < -100) m_lDelayDeltaY = -100;

		m_lDelayNumber = (DWORD)sqrt((double)abs(m_lDelayDeltaY)) * 5;
		m_lDelayLeft = m_lDelayNumber;
		m_pManager->SetTimer(this, SCROLL_TIMERID, 50U);
		return;
	}

	CListUI::DoEvent(event);
}

void CFileTransUI::DoInit()
{
	__super::DoInit();

	// 不能使用SetMouseChildEnabled(false);  不然列表中的按钮不能点击。
	// SetMouseChildEnabled(false);

	// 这里采用了一个投机的方法，将Hot和Selected状态的项背景色设置为0，而0值是不会绘制的。 [12/26/2014 ybt] 
	SetHotItemBkColor(0);
	SetSelectedItemBkColor(0);

	CListHeaderUI* pListHeader = GetHeader();
	if (pListHeader) {
		pListHeader->SetVisible(false);
	}

// 	CPaintManagerUI* pParentPM = m_pOwner->GetManager();
// 	m_pManager->UseParentResource(m_pManager);
}

bool CFileTransUI::AddSendFileObj(SendFileObj* obj)
{
	CSendFileElementUI*	pElement = NULL;
	CDialogBuilder		dlgBuilder;

	if (!dlgBuilder.GetMarkup()->IsValid())
	{
		pElement = static_cast<CSendFileElementUI*>(dlgBuilder.Create(_T("xmls\\filetrans_sendfile.xml"), 
			0, this, m_pManager));
	}
	else {
		pElement = static_cast<CSendFileElementUI*>(dlgBuilder.Create(0, m_pManager));
	}
	if (pElement == NULL) {
		return false;
	}

	pElement->SetTag((UINT_PTR)obj);
	pElement->SetOwner(this);
	obj->tag = (UINT)pElement;

	CLabelUI*	pLabelFname = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pElement, _T("label_fname")));
	if (pLabelFname != NULL) 
	{
		char	szFname[MAX_PATH];
		PathToFname(szFname, obj->fileInfo->fname);
		pLabelFname->SetText(szFname);
		pLabelFname->SetToolTip(obj->fileInfo->fname);
	}

	CLabelUI*	pLabelFsize = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pElement, _T("label_fsize")));
	if (pLabelFsize != NULL)
	{
		TCHAR szSize[30] = {0};
		//wsprintf(szSize, _T("%I64d"), obj->fileInfo->size);
		MakeSizeString(szSize, obj->fileInfo->size);
		pLabelFsize->SetText(szSize);
	}

	CButtonUI*	pBtnCancel = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(pElement, _T("btn_cancel")));
	if (pBtnCancel != NULL)
	{
		pBtnCancel->OnNotify += MakeDelegate(pElement, &CSendFileElementUI::OnBtnCancel);
	}

	if (!CListUI::Add(pElement)) 
	{
		delete pElement;
		return false;
	}
	m_nShareCount++;
	return true;
}

bool CFileTransUI::AddRecvFileObj(RecvFileObj* obj)
{
	CRecvFileElementUI*	pElement = NULL;
	CDialogBuilder		dlgBuilder;

	if (!dlgBuilder.GetMarkup()->IsValid())
	{
		pElement = static_cast<CRecvFileElementUI*>(dlgBuilder.Create(_T("xmls\\filetrans_recvfile.xml"), 
			0, this, m_pManager));
	}
	else {
		pElement = static_cast<CRecvFileElementUI*>(dlgBuilder.Create(0, m_pManager));
	}
	if (pElement == NULL) {
		return false;
	}

	pElement->SetTag((UINT_PTR)obj);
	pElement->SetOwner(this);
	obj->tag = (UINT)pElement;

	CLabelUI*	pLabelFname = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pElement, _T("label_fname")));
	if (pLabelFname != NULL) 
	{
		char	szFname[MAX_PATH];
		PathToFname(szFname, obj->fileInfo->fname);
		pLabelFname->SetText(szFname);
		pLabelFname->SetToolTip(obj->fileInfo->fname);
	}

	CLabelUI*	pLabelFsize = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pElement, _T("label_fsize")));
	if (pLabelFsize != NULL)
	{
		TCHAR szSize[30] = {0};
		MakeSizeString(szSize, obj->fileInfo->size);
		pLabelFsize->SetText(szSize);
	}

	CButtonUI*	pBtnRecv = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(pElement, _T("btn_recv")));
	if (pBtnRecv != NULL)
	{
		pBtnRecv->OnNotify += MakeDelegate(pElement, &CRecvFileElementUI::OnBtnRecv);
	}

	CButtonUI*	pBtnSaveAs = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(pElement, _T("btn_saveas")));
	if (pBtnSaveAs != NULL)
	{
		pBtnSaveAs->OnNotify += MakeDelegate(pElement, &CRecvFileElementUI::OnBtnSaveAs);
	}

	CButtonUI*	pBtnCancel = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(pElement, _T("btn_cancel")));
	if (pBtnCancel != NULL)
	{
		pBtnCancel->OnNotify += MakeDelegate(pElement, &CRecvFileElementUI::OnBtnCancel);
	}

	if (!CListUI::Add(pElement)) 
	{
		delete pElement;
		return false;
	}
	m_nShareCount++;
	return true;
}

bool CFileTransUI::RemoveShareElement(CControlUI* pElement)
{
	if (pElement && Remove(pElement))
	{
		m_nShareCount--;
		return true;
	}
	return false;
}

void CFileTransUI::RemoveAllShareElement()
{
	CControlUI*	pControl = NULL;
	CSendFileElementUI*	pSendFileElement = NULL;
	CRecvFileElementUI*	pRecvFileElement = NULL;

	for (int i = 0; i < GetCount(); i++)
	{
		pControl = GetItemAt(i);
		if (pControl->GetInterface(kRecvFileElementUIInterfaceName) != NULL)
		{
			pRecvFileElement = static_cast<CRecvFileElementUI*>(pControl);
			if (pRecvFileElement)
			{
				pRecvFileElement->EndRecvFile();
			}
			RemoveShareElement(pControl);
		}
		else if (pControl->GetInterface(kSendFileElementUIClassName) != NULL)
		{
			pSendFileElement = static_cast<CSendFileElementUI*>(pControl);
			if (pSendFileElement)
			{
				pSendFileElement->EndSendFile();
			}
			RemoveShareElement(pControl);
		}
	}
	m_nShareCount = 0;
}

LONG CFileTransUI::GetShareCount()
{
	return m_nShareCount;
}

//////////////////////////////////////////////////////////////////////////
//CSendFileElementUI
CSendFileElementUI::CSendFileElementUI()
 : m_nPercent(0)
 , m_pProgress(NULL)
 , m_pLabelSpeed(NULL)
 , m_pBtnCancel(NULL)
{

}

LPCTSTR CSendFileElementUI::GetClass() const
{
	return kSendFileElementUIClassName;
}

LPVOID CSendFileElementUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcsicmp(pstrName, kSendFileElementUIInterfaceName) == 0)
	{
		return static_cast<CSendFileElementUI*>(this);
	}
	return CListContainerElementUI::GetInterface(pstrName);
}

void CSendFileElementUI::DoInit()
{
	// __super::DoInit();
	m_pProgress = static_cast<CProgressUI*>(m_pManager->FindSubControlByName(this, _T("progress_trans")));
	m_pLabelSpeed = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("label_speed")));
	m_pBtnCancel = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(this, _T("btn_cancel")));
}

bool CSendFileElementUI::OnBtnCancel(void* param)
{
	TNotifyUI* pMsg = (TNotifyUI*)param;
	if (pMsg->sType == DUI_MSGTYPE_CLICK)
	{
		EndSendFile();
	}
	return true;
}

void CSendFileElementUI::StartSendFile()
{
	if (m_pLabelSpeed) { m_pLabelSpeed->SetVisible(true); }
	if (m_pBtnCancel) {	m_pBtnCancel->SetText(_T("中断")); }
}

void CSendFileElementUI::SetTransState()
{
	char	buf[MAX_PATH];
	int		pos;

	if (m_pLabelSpeed == NULL) return; 

	SendFileObj* sendObj = reinterpret_cast<SendFileObj*>(GetTag());

	if (sendObj->isDir && sendObj->fileInfo->size == 0)			//如果是文件夹 而且是飞鸽等发过来的文件夹
	{
		MakeDirTransRateStr(buf, sendObj->conInfo->lastTick - sendObj->conInfo->startTick,
			sendObj->offset + sendObj->totalTrans, sendObj->totalFiles);
		m_pLabelSpeed->SetText(buf);
		
		if (sendObj->status == FS_COMPLETE)  { m_nPercent = 100; }		
		else if (m_nPercent < 99){ m_nPercent++; }
		if (m_pProgress) m_pProgress->SetValue(m_nPercent);
	}
	else
	{
		pos = MakeTransRateStr(buf, sendObj->conInfo->lastTick - sendObj->conInfo->startTick,
			sendObj->offset + sendObj->totalTrans, sendObj->fileInfo->size);
		m_pLabelSpeed->SetText(buf);
		if (m_pProgress) m_pProgress->SetValue(pos);
	}
}

void CSendFileElementUI::EndSendFile()
{
	SendFileObj*	sendObj = reinterpret_cast<SendFileObj*>(GetTag());
	CShareMng*		pShareMng = Singleton<CShareMng>::getInstance();
	if (sendObj)
	{
		// 如果不是
		if (sendObj->status != FS_COMPLETE || sendObj->status != FS_ERROR) {
			sendObj->status = FS_CANCEL;
		}
		pShareMng->EndSendFile(sendObj);
	}

	//SetVisible(false);
	if( m_pOwner == NULL ) return;
	CFileTransUI*	pFileTrans = static_cast<CFileTransUI*>(m_pOwner);
	if (pFileTrans) {
		pFileTrans->RemoveShareElement(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//CRecvFileElementUI
CRecvFileElementUI::CRecvFileElementUI()
 : m_nPercent(0)
 , m_pLableFname(NULL)
 , m_pProgress(NULL)
 , m_pLabelSpeed(NULL)
 , m_pBtnRecv(NULL)
 , m_pBtnSaveAs(NULL)
 , m_pBtnCancel(NULL)
{

}

LPCTSTR CRecvFileElementUI::GetClass() const
{
	return kRecvFileElementUIClassName;
}

LPVOID CRecvFileElementUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcsicmp(pstrName, kRecvFileElementUIInterfaceName) == 0)
	{
		return static_cast<CRecvFileElementUI*>(this);
	}
	return CListContainerElementUI::GetInterface(pstrName);
}

void CRecvFileElementUI::DoInit()
{
	//__super::DoInit();		即CControlUI::OnInit()，什么事也没干，因此可以注释
	
	m_pLableFname = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("label_fname")));
	m_pProgress = static_cast<CProgressUI*>(m_pManager->FindSubControlByName(this, _T("progress_trans")));
	m_pLabelSpeed = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("label_speed")));
	m_pBtnRecv = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(this, _T("btn_recv")));
	m_pBtnSaveAs = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(this, _T("btn_saveas")));
	m_pBtnCancel = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(this, _T("btn_cancel")));
}


bool CRecvFileElementUI::OnBtnRecv(void* param)
{
	TNotifyUI*	pMsg = (TNotifyUI*)param;
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();
	if (pMsg->sType == DUI_MSGTYPE_CLICK)
	{
		RecvFileObj* recvObj = reinterpret_cast<RecvFileObj*>(GetTag());
		if (recvObj)
		{
			pShareMng->SaveFile(recvObj);
		}
	}
	return true;
}

bool CRecvFileElementUI::OnBtnSaveAs(void* param)
{
	TNotifyUI*	pMsg = (TNotifyUI*)param;
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();
	if (pMsg->sType == DUI_MSGTYPE_CLICK)
	{
		RecvFileObj* recvObj = reinterpret_cast<RecvFileObj*>(GetTag());
		if (recvObj)
		{
			TCHAR szFile[MAX_PATH] = {0};
			TCHAR szFilter[] = {
				_T("All Files\0*.*;\0")
			};

			OPENFILENAME ofn;      
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.Flags			  = OFN_READONLY;
			ofn.lStructSize       = sizeof(ofn);
			ofn.hwndOwner         = m_pManager->GetPaintWindow();
			ofn.lpstrFile         = szFile;
			ofn.nMaxFile          = sizeof(szFile);
			ofn.lpstrFilter       = szFilter;
			ofn.nFilterIndex      = 1;
			ofn.lpstrFileTitle    = NULL;
			ofn.nMaxFileTitle     = 0;
			ofn.lpstrInitialDir   = NULL;
			ofn.lpstrDefExt		  = NULL;

			if (GetSaveFileName(&ofn) == FALSE) {
				return false;
			}
			strncpy(recvObj->saveDir, szFile, MAX_PATH);

			pShareMng->SaveFile(recvObj);
		}
	}
	return true;
}

bool CRecvFileElementUI::OnBtnCancel(void* param)
{
	TNotifyUI* pMsg = (TNotifyUI*)param;
	if (pMsg->sType == DUI_MSGTYPE_CLICK)
	{
		EndRecvFile();
	}
	return true;
}

void CRecvFileElementUI::StartRecvFile()
{
	if (m_pLableFname) {
		RecvFileObj* recvObj = reinterpret_cast<RecvFileObj*>(GetTag());
		if (recvObj) {
			m_pLableFname->SetToolTip(recvObj->saveDir);
		}
	}
	if (m_pBtnRecv) { m_pBtnRecv->SetVisible(false); }
	if (m_pBtnSaveAs) { m_pBtnSaveAs->SetVisible(false); }
	if (m_pBtnCancel) { m_pBtnCancel->SetText(_T("中断")); }
	if (m_pLabelSpeed) { m_pLabelSpeed->SetVisible(true); }
}

void CRecvFileElementUI::SetTransState()
{
	char	buf[MAX_PATH];
	int		pos;

	if (m_pLabelSpeed == NULL) return; 
	RecvFileObj* recvObj = reinterpret_cast<RecvFileObj*>(GetTag());

	//如果是文件夹 而且是飞鸽等发过来的文件夹
	if (recvObj->isDir && recvObj->fileInfo->size == 0)
	{
		MakeDirTransRateStr(buf, recvObj->conInfo->lastTick - recvObj->conInfo->startTick,
			recvObj->offset + recvObj->totalTrans, recvObj->totalFiles);
		m_pLabelSpeed->SetText(buf);
		if (recvObj->status == FS_COMPLETE)  { m_nPercent = 100; }		
		else if (m_nPercent < 99) { m_nPercent++; }
		if (m_pProgress) m_pProgress->SetValue(m_nPercent);
	}
	else
	{
		pos = MakeTransRateStr(buf, recvObj->conInfo->lastTick - recvObj->conInfo->startTick,
			recvObj->offset + recvObj->totalTrans, recvObj->fileInfo->size);
		m_pLabelSpeed->SetText(buf);
		if (m_pProgress) m_pProgress->SetValue(pos);
	}
}

void CRecvFileElementUI::EndRecvFile()
{
	//SetVisible(false);
	RecvFileObj*	recvObj = reinterpret_cast<RecvFileObj*>(GetTag());
	CShareMng*		pShareMng = Singleton<CShareMng>::getInstance();
	if (recvObj)
	{
		if (recvObj->status != FS_COMPLETE || recvObj->status != FS_ERROR) {
			recvObj->status = FS_CANCEL;
		}		
		pShareMng->EndRecvFile(recvObj);	
	}

	if( m_pOwner == NULL ) return;
	CFileTransUI*	pFileTrans = static_cast<CFileTransUI*>(m_pOwner);
	if (pFileTrans) {
		pFileTrans->RemoveShareElement(this);
	}
}
} // namespace Duilib