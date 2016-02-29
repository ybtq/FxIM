#include "StdAfx.h"
#include "Utils/Config.h"
#include "MainFrame/ShareMng.h"
#include "MainFrame/MsgMng.h"

#define FILE_BIG_ALLOC	5


//////////////////////////////////////////////////////////////////////////
//Class CShareMng
//////////////////////////////////////////////////////////////////////////
CShareMng::CShareMng()
{
}

CShareMng::~CShareMng()
{
}

ShareInfo *CShareMng::CreateShare(int packetNo)
{
	// 
	if (Search(packetNo) != NULL)
		return NULL;

	ShareInfo *info = new ShareInfo(packetNo);
	m_listShareInfo.AddObj(info);

	return	info;
}


BOOL CShareMng::AddFileShare(ShareInfo *info, const char *fname, UINT exattr/* = 0*/)
{
	for (int cnt=0; cnt < info->fileCnt; cnt++)
		if (strcmp(fname, info->fileInfo[cnt]->Fname()) == 0)
			return	FALSE;

	FileInfo	*fileInfo = new FileInfo;
	if (SetFileInfo(fname, fileInfo, exattr) == FALSE)
		return	FALSE;

	if ((info->fileCnt % FILE_BIG_ALLOC) == 0)
	{
		info->fileInfo = (FileInfo **)realloc(info->fileInfo, 
			(info->fileCnt + FILE_BIG_ALLOC) * sizeof(FileInfo *));
	}
	info->fileInfo[info->fileCnt] = fileInfo;
	info->fileCnt++;

	return	TRUE;
}

BOOL CShareMng::DelFileShare(ShareInfo *info, int fileNo)
{
	if (fileNo >= info->fileCnt)
		return	FALSE;
	memmove(info->fileInfo + fileNo, info->fileInfo + fileNo +1, (--info->fileCnt - fileNo) * sizeof(FileInfo *));

	//	statDlg->Refresh();

	return	TRUE;
}

BOOL CShareMng::SetFileInfo(const char *fname, FileInfo *info, UINT exattr/* = 0*/)
{
	Config* pCfg = Singleton<Config>::getInstance();
	WIN32_FIND_DATA	fdat;

	if (GetFileInfomation(fname, &fdat) != TRUE)
		return	FALSE;

	UINT	attr = (fdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? IM_FILE_DIR : IM_FILE_REGULAR;
	attr |= (fdat.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? IM_FILE_RONLYOPT : 0;
	attr |= (fdat.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ? IM_FILE_SYSTEMOPT : 0;
	info->attr = attr;
	info->SetFname(fname);
	if (GET_MODE(info->attr) == IM_FILE_DIR)
	{
		info->size = GetFolderSize(fname);
		strncpy(pCfg->szLastOpenDir, fname, MAX_PATH);
	}
	else {
		info->size =  (_int64)fdat.nFileSizeHigh << 32 | fdat.nFileSizeLow;
		PathToDir(pCfg->szLastOpenDir, fname);
	}
	info->mtime = FileTime2UnixTime(&fdat.ftLastWriteTime);
	info->exattr = exattr;

	return	TRUE;
}

BOOL CShareMng::AddHostShare(ShareInfo *info, SendEntry *entry, int entryNum)
{
	Config* pCfg = Singleton<Config>::getInstance();
	info->host = new Host *[info->hostCnt = entryNum];
	info->transStat = new char [info->hostCnt * info->fileCnt];
	memset(info->transStat, TRANS_INIT, info->hostCnt * info->fileCnt);

	for (int cnt=0; cnt < entryNum; cnt++)
	{
		info->host[cnt] = (Host *)pCfg->fileHosts.GetHostByNameAddr(&entry[cnt].host->hostSub);
		if (info->host[cnt] == NULL)
		{
			info->host[cnt] = new Host;
			info->host[cnt]->hostSub = entry[cnt].host->hostSub;
			strncpy(info->host[cnt]->nickName, entry[cnt].host->nickName, MAX_NAMEBUF);
			pCfg->fileHosts.AddHost(info->host[cnt]);
		}
		else info->host[cnt]->RefCnt(1);
	}
	SYSTEMTIME	st;
	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &info->attachTime);

	//	statDlg->Refresh();

	return	TRUE;
}

int CShareMng::GetFileInfoNo(ShareInfo *info, FileInfo *fileInfo)
{
	for (int target=0; info->fileCnt; target++)
		if (info->fileInfo[target] == fileInfo)
			return	target;
	return	-1;
}

BOOL CShareMng::EndHostShare(int packetNo, HostSub *hostSub, FileInfo *fileInfo, BOOL done)
{
	Config* pCfg = Singleton<Config>::getInstance();
	ShareInfo *info = Search(packetNo);

	if (info == NULL)
		return	FALSE;

	for (int cnt=0; cnt < info->hostCnt; cnt++)
	{
		if (info->host[cnt]->hostSub == *hostSub)
		{
			if (fileInfo)
			{
				info->transStat[info->fileCnt * cnt + GetFileInfoNo(info, fileInfo)] = done ? TRANS_DONE : TRANS_INIT;
				if (done == FALSE)
					return	/*statDlg->Refresh(),*/ TRUE;
				for (int cnt2=0; cnt2 < info->fileCnt; cnt2++)
					if (info->transStat[info->fileCnt * cnt + cnt2] != TRANS_DONE)
						return	/*statDlg->Refresh(),*/ TRUE;
			}
			if (info->host[cnt]->RefCnt(-1) == 0)
			{
				pCfg->fileHosts.DelHost(info->host[cnt]);
				delete info->host[cnt];
			}
			memmove(info->host + cnt, info->host + cnt + 1, (--info->hostCnt - cnt) * sizeof(Host *));
			memmove(info->transStat + info->fileCnt * cnt, info->transStat + info->fileCnt * (cnt + 1), (info->hostCnt - cnt) * info->fileCnt);
			if (info->hostCnt == 0)
				DestroyShare(info);
			return	/*statDlg->Refresh(),*/ TRUE;
		}
	}
	return	FALSE;
}

void CShareMng::DestroyShare(ShareInfo *info)
{
	Config* pCfg = Singleton<Config>::getInstance();
	info->next->prior = info->prior;
	info->prior->next = info->next;

	while (info->hostCnt-- > 0)
	{
		if (info->host[info->hostCnt]->RefCnt(-1) == 0)
		{
			pCfg->fileHosts.DelHost(info->host[info->hostCnt]);
			delete info->host[info->hostCnt];
		}
	}
	delete [] info->host;
	delete [] info->transStat;

	while (info->fileCnt-- > 0)
		delete info->fileInfo[info->fileCnt];
	free(info->fileInfo);
	//	statDlg->Refresh();
}

ShareInfo *CShareMng::Search(int packetNo)
{
	ShareInfo *info= (ShareInfo *)m_listShareInfo.TopObj();
	for (; info; info= (ShareInfo *)m_listShareInfo.NextObj(info)) 
	{
		if (info->packetNo == packetNo) {
			return	info;
		}
	}
	return	NULL;
}


/*
BOOL CShareMng::GetShareCntInfo(ShareCntInfo *cntInfo, ShareInfo *shareInfo)
{

	int cnt;
	memset(cntInfo, 0, sizeof(ShareCntInfo));

	for (ShareInfo *info = shareInfo ? shareInfo : Top(); info; info=Next(info))
	{
		if (info->hostCnt)
		{
			cntInfo->packetCnt++;
			cntInfo->hostCnt += info->hostCnt;
			for (cnt=info->fileCnt * info->hostCnt -1; cnt >= 0; cnt--)
			{
				cntInfo->fileCnt++;
				switch (info->transStat[cnt]) {
				case TRANS_INIT: break;
				case TRANS_BUSY: cntInfo->transferCnt++;	break;
				case TRANS_DONE: cntInfo->doneCnt++;		break;
				}
			}
			for (cnt=0; cnt < info->fileCnt; cnt++)
			{
				if (GET_MODE(info->fileInfo[cnt]->attr) == IM_FILE_DIR)
					cntInfo->dirCnt++;
				cntInfo->totalSize += info->fileInfo[cnt]->size;
			}
		}
		if (shareInfo)
			return	TRUE;
	}
	return	TRUE;
}*/


BOOL CShareMng::GetAcceptableFileInfo(ConnectInfo *info, char *buf, AcceptFileInfo *fileInfo)
{
	char		*tok, *p;
	int			targetID;
	ShareInfo	*shareInfo;
	HostSub		hostSub = { "", "", info->addr, info->port };

	if ((tok = separate_token(buf, ':', &p)) == NULL || atoi(tok) != IM_VERSION)
		return	FALSE;

	if ((tok = separate_token(NULL, ':', &p)) == NULL)	// packet no
		return	FALSE;

	if ((tok = separate_token(NULL, ':', &p)) == NULL)
		return	FALSE;
	strncpy(hostSub.userName, tok, MAX_NAMEBUF);

	if ((tok = separate_token(NULL, ':', &p)) == NULL)
		return	FALSE;
	strncpy(hostSub.hostName, tok, MAX_NAMEBUF);

	if ((tok = separate_token(NULL, ':', &p)) == NULL)	// command
		return	FALSE;
	fileInfo->command = atoi(tok);

	if ((tok = separate_token(NULL, ':', &p)) == NULL)
		return	FALSE;
	fileInfo->packetNo = strtol(tok, 0, 16);

	if ((tok = separate_token(NULL, ':', &p)) == NULL)
		return	FALSE;
	targetID = strtol(tok, 0, 16);

	if (GET_MODE(fileInfo->command) == IM_GETFILEDATA)
	{
		if ((tok = separate_token(NULL, ':', &p)) == NULL)
			return	FALSE;
		fileInfo->offset = hex2ll(tok);
	}
	else if (GET_MODE(fileInfo->command) == IM_GETDIRFILES)
		fileInfo->offset = 0;
	else return	FALSE;

	if ((shareInfo = Search(fileInfo->packetNo)) == NULL)
		return	FALSE;

	int	host_cnt, file_cnt;

	for (host_cnt=0; host_cnt < shareInfo->hostCnt; host_cnt++)
	{
		if (hostSub == shareInfo->host[host_cnt]->hostSub)
		{
			fileInfo->host = shareInfo->host[host_cnt];
			break;
		}
	}
	if (host_cnt == shareInfo->hostCnt)
		return	FALSE;

	for (file_cnt=0; file_cnt < shareInfo->fileCnt; file_cnt++)
	{
		if (shareInfo->fileInfo[file_cnt]->id == targetID)
		{
			fileInfo->fileInfo = shareInfo->fileInfo[file_cnt];
			if (shareInfo->transStat[shareInfo->fileCnt * host_cnt + file_cnt] != TRANS_INIT)
				return	FALSE;
			if (GET_MODE(fileInfo->command) != IM_GETDIRFILES
				&& GET_MODE(fileInfo->fileInfo->attr) == IM_FILE_DIR)		// dir 
				return	FALSE;
			fileInfo->attachTime = shareInfo->attachTime;
			shareInfo->transStat[shareInfo->fileCnt * host_cnt + file_cnt] = TRANS_BUSY;
			//statDlg->Refresh();
			return	TRUE;
		}
	}
	return	FALSE;
}



BOOL CShareMng::AddConnectInfo(ConnectInfo* conInfo)
{
	int	cnt;
	ShareInfo *info= (ShareInfo *)m_listShareInfo.TopObj();

	for (; info; info=(ShareInfo *)m_listShareInfo.NextObj(info))
	{
		for (cnt=0; cnt < info->hostCnt; cnt++)
		{
			if (info->host[cnt]->hostSub.addr == conInfo->addr/* && info->host[cnt]->hostSub.portNo == conInfo->port*/)
			{
				conInfo->port = info->host[cnt]->hostSub.portNo;
				m_listConInfo.AddObj(conInfo);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CShareMng::RemoveConnectInfo(ConnectInfo* conInfo)
{
	ConnectInfo* info = (ConnectInfo*)m_listConInfo.TopObj();
	for (; info; info = (ConnectInfo*)m_listConInfo.NextObj(info))
	{
		if (info == conInfo)
		{
			m_listConInfo.DelObj(conInfo);
			return TRUE;
		}
	}
	return FALSE;
}

ConnectInfo* CShareMng::GetConnectInfo(SOCKET sd)
{
	ConnectInfo* info = (ConnectInfo*)m_listConInfo.TopObj();
	for (; info; info = (ConnectInfo*)m_listConInfo.NextObj(info))
	{
		if (info->sd == sd)
		{
			return info;
		}
	}
	return NULL;
}

BOOL CShareMng::AddSendFileObjs(ShareInfo* shareInfo, HWND hWndTrans)
{
	int	i, j;

	if (shareInfo == NULL || shareInfo->fileCnt == 0 || shareInfo->hostCnt == 0) 
	{
		return FALSE;
	}
	for (i = 0; i < shareInfo->fileCnt; i++)
	{
		for (j = 0; j < shareInfo->hostCnt; j++)
		{
			SendFileObj* sendObj = new SendFileObj;
			if (sendObj)
			{
				memset(sendObj, 0, (BYTE*)&sendObj->fdata - (BYTE*)sendObj);
				memset(&sendObj->fdata, 0, sizeof(WIN32_FIND_DATA));
				sendObj->fileInfo = shareInfo->fileInfo[i];
				sendObj->packetNo = shareInfo->packetNo;
				sendObj->hWndTrans = hWndTrans;
				sendObj->host = shareInfo->host[j];
				m_listSendFileObj.AddObj(sendObj);
			}
		}
	}
	return TRUE;
}

SendFileObj* CShareMng::GetSendFileObj(FileInfo *fileInfo)
{
	SendFileObj* sendObj = (SendFileObj*)m_listSendFileObj.TopObj();
	for ( ; sendObj; sendObj = (SendFileObj*)m_listSendFileObj.NextObj(sendObj))
	{
		if (sendObj->fileInfo == fileInfo)
		{
			return sendObj;
		}
	}
	return NULL;
}

SendFileObj* CShareMng::GetSendFileObj(SOCKET sd)
{
	SendFileObj* sendObj = (SendFileObj*)m_listSendFileObj.TopObj();
	for ( ; sendObj; sendObj = (SendFileObj*)m_listSendFileObj.NextObj(sendObj))
	{
		if (sendObj->conInfo && sendObj->conInfo->sd == sd)
		{
			return sendObj;
		}
	}
	return NULL;
}

BOOL CShareMng::StartSendFile(SOCKET sd, HWND hWndTcp)
{
	char			buf[MAX_PATH];
	int				size;
	Config*			pCfg = Singleton<Config>::getInstance();
	CMsgMng*		pMsgMng = Singleton<CMsgMng>::getInstance();
	ConnectInfo 	*connectInfo = NULL;
	AcceptFileInfo	acceptFileInfo;

	//找到对应的连接信息
	if ((connectInfo = GetConnectInfo(sd)) == NULL)
	{
		::closesocket(sd);
		return FALSE;
	}
	pMsgMng->ConnectDone(hWndTcp, connectInfo);

	if ((size = ::recv(connectInfo->sd, buf, sizeof(buf) -1, 0)) > 0){
		buf[size] = 0;									//最后一位置'\0'
	}
	if (size <= 0 || GetAcceptableFileInfo(connectInfo, buf, &acceptFileInfo) == FALSE)
	{
		RemoveConnectInfo(connectInfo);					//删除连接
		::closesocket(connectInfo->sd);
		delete connectInfo;
		connectInfo = NULL;
		return	FALSE;
	}

	//找到对应的CSendFileObj
	SendFileObj* sendObj = GetSendFileObj(acceptFileInfo.fileInfo);
	if (sendObj == NULL) {
		return FALSE;
	}
	sendObj->hWndTcp = hWndTcp;
	sendObj->packetNo = acceptFileInfo.packetNo;
	sendObj->attachTime = acceptFileInfo.attachTime;
	sendObj->offset = acceptFileInfo.offset;
	sendObj->host = acceptFileInfo.host;
	sendObj->command = acceptFileInfo.command;
	connectInfo->startTick = connectInfo->lastTick = ::GetTickCount();
	sendObj->conInfo = connectInfo; 
	RemoveConnectInfo(connectInfo);			// 只删除，不释放空间

	if (!GetFileInfomation(sendObj->fileInfo->Fname(), &sendObj->fdata)){
		EndSendFile(sendObj);
		return FALSE;
	}

	sendObj->isDir = (sendObj->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
	sendObj->status = sendObj->isDir || sendObj->command == IM_GETDIRFILES ? FS_DIRFILESTART : FS_TRANSFILE;

	if (*sendObj->fdata.cFileName == 0)
		ForcePathToFname(sendObj->fdata.cFileName, sendObj->fileInfo->Fname());

	BOOL	ret;
	if (sendObj->isDir)
	{
		ret = sendObj->command == IM_GETDIRFILES ? TRUE : FALSE;
		sendObj->hDir = (HANDLE *)malloc((MAX_PATH/2) * sizeof(HANDLE));
	}
	else {
		if ((pCfg->fileTransOpt & FT_STRICTDATE) 
			&& *(_int64 *)&sendObj->fdata.ftLastWriteTime > *(_int64 *)&sendObj->attachTime)
			ret = FALSE, sendObj->status = FS_COMPLETE;
		else if (sendObj->command == IM_GETDIRFILES)
			ret = TRUE;
		else
			ret = OpenSendFile(sendObj, sendObj->fileInfo->Fname());
	}
	if (ret == FALSE)
		return	EndSendFile(sendObj), FALSE;

	DWORD	id;
	sendObj->hThread = (HANDLE)~0;
	
	if ((sendObj->hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendFileThread,
		sendObj, 0, &id)) == NULL)
	{
		return	EndSendFile(sendObj), FALSE;
	}

	if (sendObj->hWndTrans) {
		PostMessage(sendObj->hWndTrans, WM_SENDOBJEVENT, (WPARAM)sendObj, (LPARAM)TRANS_INIT);
	}
	
	return TRUE;
}

BOOL CShareMng::OpenSendFile(SendFileObj *sendObj, const char *fname)
{
	DWORD	lowSize, highSize, viewSize;
	Config* pCfg = Singleton<Config>::getInstance();

	if ((sendObj->hFile = ::CreateFile(fname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE)
	{
		lowSize = ::GetFileSize(sendObj->hFile, &highSize);
		if ((sendObj->fileSize = (_int64)highSize << 32 | lowSize) == 0)
			return	TRUE;
		sendObj->hMap = ::CreateFileMapping(sendObj->hFile, 0, PAGE_READONLY, highSize, lowSize, 0);
		viewSize = (int)(sendObj->fileSize > pCfg->ViewMax ? pCfg->ViewMax : sendObj->fileSize);
		highSize = (int)(sendObj->offset >> 32);
		lowSize = (int)((sendObj->offset / pCfg->ViewMax) * pCfg->ViewMax);
		sendObj->mapAddr = (char *)::MapViewOfFile(sendObj->hMap, FILE_MAP_READ, 
			highSize, lowSize, viewSize);
		if (sendObj->mapAddr && IsBadReadPtr(sendObj->mapAddr, 1))
		{
			CloseSendFile(sendObj);
			return	FALSE;
		}
	}
	return	sendObj->mapAddr ? TRUE : FALSE;
}

BOOL CShareMng::EndSendFile(SendFileObj* sendObj)
{
	if (sendObj == NULL)
		return	FALSE;

	if (sendObj->hThread)
	{
		HANDLE	hThread = sendObj->hThread;
		sendObj->hThread = 0;
		::WaitForSingleObject(hThread, INFINITE);
		::CloseHandle(hThread);
	}
	if (sendObj->conInfo == NULL || ::closesocket(sendObj->conInfo->sd) != 0)
		sendObj->status = FS_ERROR;	// error 
	CloseSendFile(sendObj);

	if (sendObj->isDir)
		free(sendObj->hDir);

	if (sendObj->hWndTrans) {
		SendMessage(sendObj->hWndTrans, WM_SENDOBJEVENT, (WPARAM)sendObj, 
			(LPARAM)CShareMng::TRANS_DONE);
	}

	EndHostShare(sendObj->packetNo, &sendObj->host->hostSub, sendObj->fileInfo,
		sendObj->status == FS_COMPLETE ? TRUE : FALSE);
	m_listSendFileObj.DelObj(sendObj);
	if (sendObj->conInfo != NULL) {
		delete sendObj->conInfo;
	}
	delete sendObj;
	return	TRUE;
}

BOOL CShareMng::CloseSendFile(SendFileObj* sendObj)
{
	if (sendObj == NULL)
		return	FALSE;

	::UnmapViewOfFile(sendObj->mapAddr);sendObj->mapAddr= NULL;
	::CloseHandle(sendObj->hMap);		sendObj->hMap	= NULL;
	::CloseHandle(sendObj->hFile);		sendObj->hFile	= INVALID_HANDLE_VALUE;
	sendObj->totalTrans += sendObj->offset;
	sendObj->totalFiles++;
	sendObj->offset = 0;

	return	TRUE;
}

int	CShareMng::MakeDirHeader(SendFileObj *sendObj, BOOL find)
{
	int				len;
	WIN32_FIND_DATA	*dat = &sendObj->fdata;
	DWORD			attr = dat->dwFileAttributes, ipmsg_attr;

	ipmsg_attr = (find == FALSE ? IM_FILE_RETPARENT : (attr & FILE_ATTRIBUTE_DIRECTORY)
		? IM_FILE_DIR : IM_FILE_REGULAR) |
		(attr & FILE_ATTRIBUTE_READONLY ? IM_FILE_RONLYOPT : 0) |
		(attr & FILE_ATTRIBUTE_HIDDEN ? IM_FILE_HIDDENOPT : 0) |
		(attr & FILE_ATTRIBUTE_SYSTEM ? IM_FILE_SYSTEMOPT : 0);

	if (find)
		len = wsprintf(sendObj->header, "0000:%s:%x%08x:%x:%x=%x:%x=%x:", dat->cFileName,
		dat->nFileSizeHigh, dat->nFileSizeLow, ipmsg_attr,
		IM_FILE_MTIME, FileTime2UnixTime(&dat->ftLastWriteTime),
		IM_FILE_CREATETIME, FileTime2UnixTime(&dat->ftCreationTime));
	else if (*(_int64 *)&dat->ftLastWriteTime)
		len = wsprintf(sendObj->header, "0000:.:0:%x:%x=%x:%x=%x:", ipmsg_attr,
		IM_FILE_MTIME, FileTime2UnixTime(&dat->ftLastWriteTime),
		IM_FILE_CREATETIME, FileTime2UnixTime(&dat->ftCreationTime));
	else
		len = wsprintf(sendObj->header, "0000:.:0:%x:", ipmsg_attr);

	sendObj->header[wsprintf(sendObj->header, "%04x", len)] = ':';

	return	len;
}

BOOL CShareMng::SendFile(SendFileObj* sendObj)
{
	if (sendObj == NULL || sendObj->hFile == INVALID_HANDLE_VALUE)
		return	FALSE;
	Config* pCfg = Singleton<Config>::getInstance();

	int		size = 0;
	_int64	remain = sendObj->fileSize - sendObj->offset;
	int		transMax = pCfg->TransMax - (int)(sendObj->offset % pCfg->TransMax);

	if (remain > 0 && (size = ::send(sendObj->conInfo->sd, 
		sendObj->mapAddr + (sendObj->offset % pCfg->ViewMax), 
		(int)(remain > transMax ? transMax : remain), 0)) < 0)
	{
		return	FALSE;
	}

	sendObj->offset += size;

	if (sendObj->offset == sendObj->fileSize)
		sendObj->status = sendObj->command == IM_GETDIRFILES ? FS_ENDFILE : FS_COMPLETE;
	else if ((sendObj->offset % pCfg->ViewMax) == 0)
	{
		::UnmapViewOfFile(sendObj->mapAddr);
		remain = sendObj->fileSize - sendObj->offset;
		sendObj->mapAddr = (char *)::MapViewOfFile(sendObj->hMap, FILE_MAP_READ,
			(int)(sendObj->offset >> 32), (int)sendObj->offset, 
			(int)(remain > pCfg->ViewMax ? pCfg->ViewMax : remain));
	}

	// 此处有待考察是否有必要。
	DWORD nowTick = ::GetTickCount();
	if (nowTick - sendObj->conInfo->lastTick >= 1000)
	{
		sendObj->conInfo->lastTick = ::GetTickCount();
		if (sendObj->hWndTrans) {
			PostMessage(sendObj->hWndTrans, WM_SENDOBJEVENT, (WPARAM)sendObj, (LPARAM)TRANS_BUSY);
		}
	}

	return	TRUE;
}

BOOL CShareMng::SendDirFile(SendFileObj *sendObj)
{
	BOOL	find = FALSE;
	Config* pCfg = Singleton<Config>::getInstance();

	if (sendObj->status == FS_OPENINFO)
	{
		char	buf[MAX_BUF];
		if (sendObj->dirCnt == 0)
		{
			strncpy(buf, sendObj->fileInfo->Fname(), MAX_PATH);
		}
		else if (MakePath(buf, sendObj->path, *sendObj->fdata.cAlternateFileName ? 
			sendObj->fdata.cAlternateFileName : sendObj->fdata.cFileName) >= MAX_PATH)
		{
			return	FALSE;
		}
		strncpy(sendObj->path, buf, MAX_PATH);
		sendObj->dirCnt++;
		sendObj->status = FS_FIRSTINFO;
	}

	if (sendObj->status == FS_FIRSTINFO || sendObj->status == FS_NEXTINFO)
	{
		if (sendObj->status == FS_FIRSTINFO)
		{
			char	buf[MAX_BUF];
			MakePath(buf, sendObj->path, "*");
			find = (sendObj->hDir[sendObj->dirCnt -1] = 
				::FindFirstFile(buf, &sendObj->fdata)) == INVALID_HANDLE_VALUE ? FALSE : TRUE;
		}
		else find = ::FindNextFile(sendObj->hDir[sendObj->dirCnt -1], &sendObj->fdata);

		while (find && (strcmp(sendObj->fdata.cFileName, ".") == 0 
			|| strcmp(sendObj->fdata.cFileName, "..") == 0))
		{
			find = ::FindNextFile(sendObj->hDir[sendObj->dirCnt -1], &sendObj->fdata);
		}
		sendObj->status = FS_MAKEINFO;
	}

	if (sendObj->status == FS_DIRFILESTART || sendObj->status == FS_MAKEINFO)
	{
		if (sendObj->status == FS_DIRFILESTART)
			find = TRUE;
		if (find && (sendObj->dirCnt > 0 || sendObj->isDir == FALSE) 
			&& (sendObj->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			char	buf[MAX_BUF];
			int		len = sendObj->isDir ? MakePath(buf, sendObj->path, 
						*sendObj->fdata.cAlternateFileName ? sendObj->fdata.cAlternateFileName 
						: sendObj->fdata.cFileName) : wsprintf(buf, "%s", sendObj->fileInfo->Fname());
			BOOL	modifyCheck = (pCfg->fileTransOpt & FT_STRICTDATE) 
						&& *(_int64 *)&sendObj->fdata.ftLastWriteTime > *(_int64 *)&sendObj->attachTime;
			if (len >= MAX_PATH || modifyCheck || OpenSendFile(sendObj, buf) == FALSE)
			{
				len = strlen(sendObj->fdata.cFileName);
				strncpy(sendObj->fdata.cFileName + len, " (Can't open)", MAX_PATH - len);
				sendObj->fdata.nFileSizeHigh = sendObj->fdata.nFileSizeLow = 0;
			}
		}
		if (find == FALSE && sendObj->isDir)
			GetFileInfomation(sendObj->path, &sendObj->fdata);

		sendObj->headerOffset = 0;
		sendObj->headerLen = MakeDirHeader(sendObj, find);
		if (find == FALSE)
		{
			if (--sendObj->dirCnt >= 0 && sendObj->isDir)
			{
				::FindClose(sendObj->hDir[sendObj->dirCnt]);
				if (PathToDir(sendObj->path, sendObj->path) != TRUE && sendObj->dirCnt > 0)
					return	FALSE;
			}
			if (sendObj->dirCnt <= 0)
				sendObj->dirCnt--;	// 廔椆
		}
		sendObj->status = FS_TRANSINFO;
	}

	if (sendObj->status == FS_TRANSINFO)
	{
		int	size = ::send(sendObj->conInfo->sd, sendObj->header + sendObj->headerOffset, 
			sendObj->headerLen - sendObj->headerOffset, 0);
		if (size < 0)
			return	FALSE;
		else {
			if ((sendObj->headerOffset += size) < sendObj->headerLen)
				return	TRUE;
			sendObj->status = sendObj->dirCnt < 0 ? FS_COMPLETE : find == FALSE ? FS_NEXTINFO : 
				(sendObj->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FS_OPENINFO : FS_TRANSFILE;
		}
	}

	if (sendObj->status == FS_TRANSFILE)
	{
		if (sendObj->mapAddr && SendFile(sendObj) != TRUE)
		{
			CloseSendFile(sendObj);
			return	FALSE;
		}
		else if (sendObj->mapAddr == NULL || sendObj->status == FS_ENDFILE)
		{
			CloseSendFile(sendObj);
			sendObj->status = sendObj->isDir ? FS_NEXTINFO : FS_MAKEINFO;
		}
	}
	return	TRUE;
}



DWORD CShareMng::SendFileThread(void *obj)
{
	SendFileObj	*sendObj = (SendFileObj *)obj;
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();
	fd_set		fds;
	fd_set		*rfds = NULL, *wfds = &fds;
	timeval		tv;
	int			sock_ret;
	BOOL		ret = FALSE, completeWait = FALSE;
	BOOL		(CShareMng::*SendFileFunc)(SendFileObj *sendObj) =
					(sendObj->command == IM_GETDIRFILES) ? &CShareMng::SendDirFile : &CShareMng::SendFile;

	FD_ZERO(&fds);
	FD_SET(sendObj->conInfo->sd, &fds);

	for (int waitCnt=0; waitCnt < 180 && sendObj->hThread != NULL; waitCnt++)
	{
		tv.tv_sec = 1, tv.tv_usec = 0;

		if ((sock_ret = ::select(sendObj->conInfo->sd + 1, rfds, wfds, NULL, &tv)) > 0)
		{
			waitCnt = 0;

			if (completeWait)
			{
				if (::recv(sendObj->conInfo->sd, (char *)&ret, sizeof(ret), 0) >= 0)
					ret = TRUE;
				break;
			}
			else if ((pShareMng->*SendFileFunc)(sendObj) != TRUE)
				break;
			else if (sendObj->status == FS_COMPLETE)
			{
				completeWait = TRUE, rfds = &fds, wfds = NULL;
				// 
				if (sendObj->fileSize == 0) { ret = TRUE; break; }
			}
		}
		else if (sock_ret == 0) {
			FD_ZERO(&fds);
			FD_SET(sendObj->conInfo->sd, &fds);
			sendObj->conInfo->lastTick = ::GetTickCount();
			if (sendObj->hWndTrans) {
				PostMessage(sendObj->hWndTrans, WM_SENDOBJEVENT, (WPARAM)sendObj, (LPARAM)TRANS_BUSY);
			}
		}
		else if (sock_ret == SOCKET_ERROR) {
			DUI__Trace(_T("SendFileThread select failed, error code : %d\n"), WSAGetLastError());
			break;
		}
	}

	if (sendObj->isDir)
	{
		pShareMng->CloseSendFile(sendObj);
		while (--sendObj->dirCnt >= 0)
			::FindClose(sendObj->hDir[sendObj->dirCnt]);
	}

	sendObj->status = ret ? FS_COMPLETE : FS_ERROR;
	sendObj->conInfo->lastTick = ::GetTickCount();

	//pShareMng->EndSendFile(sendObj, FALSE);
	PostMessage(sendObj->hWndTcp, WM_TCPEVENT, sendObj->conInfo->sd, FD_CLOSE);
	::ExitThread(0);
	return	0;
}

//////////////////////////////////////////////////////////////////////////
// File Recv.
BOOL CShareMng::AddRecvFileObjs(ShareInfo* shareInfo, HWND hWndTcp, HWND hWndTrans, 
							   ULONG addr, USHORT port)
{
	Config* pCfg = Singleton<Config>::getInstance();
	if (shareInfo == NULL || shareInfo->fileCnt <= 0) {
		return FALSE;
	}
	for (int i = 0; i < shareInfo->fileCnt; i++)
	{
		RecvFileObj* recvObj = new RecvFileObj;

		if (recvObj)
		{
			memset(recvObj, 0, sizeof(RecvFileObj));
			recvObj->fileInfo = new FileInfo;
			*recvObj->fileInfo = *shareInfo->fileInfo[i];				// 拷贝一份
			recvObj->hWndTrans = hWndTrans;
			recvObj->hWndTcp = hWndTcp;
			recvObj->packetNo = shareInfo->packetNo;
			recvObj->hFile = INVALID_HANDLE_VALUE;
			strncpy(recvObj->saveDir, pCfg->szDefaultFileSaveDir, MAX_PATH);
			recvObj->conInfo = new ConnectInfo;
			memset(recvObj->conInfo, 0, sizeof(ConnectInfo));
			recvObj->conInfo->addr = addr;
			recvObj->conInfo->port = port;

			m_listRecvFileObj.AddObj(recvObj);
		}
	}
	return TRUE;
}

RecvFileObj* CShareMng::GetRecvFileObj(SOCKET sd)
{
	RecvFileObj* recvObj = NULL;
	for (recvObj = TopRecvObj(); recvObj; recvObj = NextRecvObj(recvObj))
	{
		if (recvObj->conInfo && recvObj->conInfo->sd == sd)
		{
			return recvObj;
		}
	}
	return NULL;
}

void CShareMng::SaveRecvFileObjs(ULONG nPacketNo, LPCTSTR szSaveDir)
{
	RecvFileObj* recvObj = NULL;
	for (recvObj = TopRecvObj(); recvObj; recvObj = NextRecvObj(recvObj))
	{
		if (recvObj->packetNo == nPacketNo)
		{
			_tcsncpy(recvObj->saveDir, szSaveDir, MAX_PATH);
			SaveFile(recvObj, TRUE);
		}
	}
}

BOOL CShareMng::RecvFile(RecvFileObj* recvObj)
{
	Config* pCfg = Singleton<Config>::getInstance();
	int		wresid = (int)(recvObj->offset - recvObj->woffset);
	_int64	remain = recvObj->curFileInfo.size - recvObj->offset;
	int		size = 0;

	if (remain > pCfg->TransMax - wresid)
		remain = pCfg->TransMax - wresid;

	if ((size = ::recv(recvObj->conInfo->sd, recvObj->recvBuf + wresid, (int)remain, 0)) <= 0)
		return DUI__Trace(_T("RecvFile recv failed. error %d"), WSAGetLastError()),	FALSE;

	if (recvObj->hFile == INVALID_HANDLE_VALUE)
		if (OpenRecvFile(recvObj) == FALSE)
			return DUI__Trace(_T("OpenRecvFile failed.")), FALSE;

	wresid += size;
	if (recvObj->offset + size >= recvObj->curFileInfo.size || pCfg->TransMax == wresid)
	{
		DWORD	wsize;
		if (::WriteFile(recvObj->hFile, recvObj->recvBuf, wresid, &wsize, 0) != TRUE 
			|| wresid != (int)wsize)
		{
			return ::MessageBox(NULL, MSGWRITEFAIL_STR, MSGCAPTION_STR, MB_ICONERROR | MB_OK), FALSE;
		}
		recvObj->woffset += wresid;
	}
	recvObj->offset += size;

	DWORD	nowTick = ::GetTickCount();

	if (nowTick - recvObj->conInfo->lastTick >= 1000)
	{
		recvObj->conInfo->lastTick = nowTick;
		if (recvObj->hWndTrans)	{
			PostMessage(recvObj->hWndTrans, WM_RECVOBJEVENT, (WPARAM)recvObj, (LPARAM)TRANS_BUSY);
		}
	}

	if (recvObj->offset >= recvObj->curFileInfo.size)
		recvObj->status = recvObj->isDir ? FS_ENDFILE : FS_COMPLETE;

	return	TRUE;
}

BOOL CShareMng::RecvDirFile(RecvFileObj* recvObj)
{
#define BIG_ALLOC	50
#define PEEK_SIZE	8

	if (recvObj->status == FS_DIRFILESTART || recvObj->status == FS_TRANSINFO)
	{
		int		size;
		if (recvObj->infoLen == 0)
		{
			if ((size = ::recv(recvObj->conInfo->sd, recvObj->info + (int)recvObj->offset, 
				PEEK_SIZE - (int)recvObj->offset, 0)) <= 0)
			{
				return	FALSE;
			}
			if ((recvObj->offset += size) < PEEK_SIZE)
				return	TRUE;
			recvObj->info[recvObj->offset] = 0;
			if ((recvObj->infoLen = strtoul(recvObj->info, 0, 16)) >= sizeof(recvObj->info) -1 
				|| recvObj->infoLen <= 0)
			{
				return	FALSE;	// too big or small
			}
		}
		if (recvObj->offset < recvObj->infoLen)
		{
			if ((size = ::recv(recvObj->conInfo->sd, recvObj->info + (int)recvObj->offset,
				recvObj->infoLen - (int)recvObj->offset, 0)) <= 0)
			{
				return	FALSE;
			}
			recvObj->offset += size;
		}
		if (recvObj->offset == recvObj->infoLen)
		{
			recvObj->info[recvObj->infoLen] = 0;
			if (DecodeDirEntry(&recvObj->curFileInfo, recvObj->info) == FALSE)
				return	FALSE;	// Illegal entry
			recvObj->offset = recvObj->infoLen = 0;	

			if (GET_MODE(recvObj->curFileInfo.attr) == IM_FILE_DIR)
			{
				char	buf[MAX_BUF];
				const char *fname = recvObj->dirCnt == 0 ?
					recvObj->fileInfo->Fname() : recvObj->curFileInfo.Fname();

				if (MakePath(buf, recvObj->path, fname) >= MAX_PATH)
					return FALSE;
				if (IsSafePath(buf, fname) == FALSE)
					return FALSE;

				if (::CreateDirectory(buf, NULL) == FALSE)
					return FALSE;
				strncpy(recvObj->path, buf, MAX_PATH);
				recvObj->dirCnt++;
			}
			else if (GET_MODE(recvObj->curFileInfo.attr) == IM_FILE_RETPARENT)
			{
				if (recvObj->curFileInfo.mtime)
				{
					FILETIME	ft;
					HANDLE		hFile;
					UnixTime2FileTime(recvObj->curFileInfo.mtime, &ft);
					if ((hFile = ::CreateFile(recvObj->path, GENERIC_WRITE, FILE_SHARE_READ,
						0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) != INVALID_HANDLE_VALUE)
					{
						::SetFileTime(hFile, NULL, NULL, &ft);
						::CloseHandle(hFile);
					}
				}
				if (recvObj->curFileInfo.attr & IM_FILE_RONLYOPT)
					::SetFileAttributes(recvObj->path, FILE_ATTRIBUTE_READONLY);
				if (--recvObj->dirCnt <= 0)
				{
					recvObj->status = FS_COMPLETE;
					return	TRUE;
				}
				if (PathToDir(recvObj->path, recvObj->path) == FALSE)
					return	FALSE;
			}
			else {
				if (recvObj->dirCnt == 0)
					return	FALSE;

				if (recvObj->curFileInfo.size == 0)	// 0byte file
				{
					if (OpenRecvFile(recvObj))		// 0byte
						CloseRecvFile(recvObj, TRUE);
				}
				recvObj->status = recvObj->curFileInfo.size ? FS_TRANSFILE : FS_TRANSINFO;
			}
			return	TRUE;
		}
	}

	if (recvObj->status == FS_TRANSFILE)
	{
		if (RecvFile(recvObj) != TRUE)
		{
			CloseRecvFile(recvObj, FALSE);
			return	FALSE;
		}
		if (recvObj->status == FS_ENDFILE)
		{
			CloseRecvFile(recvObj, TRUE);
			recvObj->status = FS_TRANSINFO;
		}
	}

	return	TRUE;
}


BOOL CShareMng::SaveFile(RecvFileObj* recvObj, BOOL bFailIfExists/* = FALSE*/)
{
// 	int		target;
// 	for (target=0; target < recvObj->shareInfo->fileCnt; target++)
// 		if (recvObj->shareInfo->fileInfo[target]->isSelected)
// 			break;
// 	if (target == recvObj->shareInfo->fileCnt)
// 		return	FALSE;

	// 已改成SaveFile之前分配空间
//	memset(recvObj, 0, (char *)&recvObj->totalTrans - (char *)recvObj);
//	recvObj->conInfo = new ConnectInfo;
// 	memset(recvObj->conInfo, 0, sizeof(ConnectInfo));
// 	recvObj->conInfo->addr = addr;
// 	recvObj->conInfo->port = port;

	//recvObj->fileInfo = recvObj->shareInfo->fileInfo[target];
	recvObj->isDir = GET_MODE(recvObj->fileInfo->attr) == IM_FILE_DIR ? TRUE : FALSE;
	recvObj->status = recvObj->isDir ? FS_DIRFILESTART : FS_TRANSFILE;
	recvObj->hFile = INVALID_HANDLE_VALUE;
	if (recvObj->isDir)
		strncpy(recvObj->path, recvObj->saveDir, MAX_PATH);

	TCHAR path[MAX_PATH] = {0};
	MakePath(path, recvObj->saveDir, recvObj->fileInfo->Fname());
	if (GetFileAttributes(path) != 0xFFFFFFFF)		//如果文件（夹）已存在
	{
		if (bFailIfExists) {
			recvObj->status = FS_ALREADYEXISTS;
			DUI__Trace(_T("CShareMng::SaveFile File is exists."));
			EndRecvFile(recvObj);
			return FALSE;
		}
		else {
			TCHAR szText[MAX_PATH * 2] = {0};
			wsprintf(szText, _T("\"%s\" 已存在，是否替换?"), path);
			if(IDOK != ::MessageBox(recvObj->hWndTrans, szText, MSGCAPTION_STR, MB_OKCANCEL))
			{
				recvObj->status = FS_ALREADYEXISTS;
				DUI__Trace(_T("CShareMng::SaveFile not replace file."));
				EndRecvFile(recvObj);
				return FALSE;
			}
		}
	}
	DUI__Trace(_T("CShareMng::SaveFile recv file"));

	if (!ConnectRecvFile(recvObj))
	{
		delete recvObj->conInfo;
		recvObj->conInfo = NULL;
		recvObj->status = FS_ERROR;
		return FALSE;
	}
	DUI__Trace(_T("CShareMng::SaveFile ConnectRecvFile success."));
	return	TRUE;
}

BOOL CShareMng::ConnectRecvFile(RecvFileObj* recvObj)
{
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	if (pMsgMng->Connect(recvObj->hWndTcp, recvObj->conInfo) != TRUE) {
		DUI__Trace(_T("CShareMng::ConnectRecvFile Connect sender failed."));
		return	FALSE;
	}

	//2011年第一版的测试结果：
	//多次测试，本软件作为接收方往往在用户点了接收后还没开始接收就已经中断传输了，而单步调试却能成功。
	//睡了一觉，猛然醒悟，原来是单步调试花的时间要长，Connect之后并不是马上StartRecvFile，
	//因为StartRecvFile里向发送方发送要接收的文件信息，所以加上下面的延迟
	//因为不加的话，接收方连接发送方，发送方的FD_ACCPET还没处理。
	//接着StartRecvFile就把信息就发过去了，发送方的FD_CLOSE估计就出问题了
	Sleep(100);	

	if (recvObj->conInfo->complete) {
		DUI__Trace(_T("CShareMng::ConnectRecvFile StartRecvFile"));
		return StartRecvFile(recvObj);
	}

	return	FALSE;
}

BOOL CShareMng::StartRecvFile(RecvFileObj* recvObj)
{
#define	OFFSET 0
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	char		buf[MAX_PATH], tcpbuf[MAX_BUF];
	wsprintf(buf, recvObj->isDir ? "%x:%x:" : "%x:%x:%x:", 
		recvObj->packetNo, recvObj->fileInfo->id, OFFSET);
	recvObj->conInfo->complete = TRUE;
	pMsgMng->MakeMsg(tcpbuf, recvObj->isDir ? IM_GETDIRFILES : IM_GETFILEDATA, buf);
	pMsgMng->ConnectDone(recvObj->hWndTcp, recvObj->conInfo);

	//recvObj->offset = recvObj->woffset = OFFSET;

	if (::send(recvObj->conInfo->sd, tcpbuf, strlen(tcpbuf), 0) < (int)strlen(tcpbuf))
	{
		DUI__Trace(_T("CShareMng::StartRecvFile send fileinfo failed, error code %d."), WSAGetLastError());
		return	EndRecvFile(recvObj), FALSE;
	}

	recvObj->conInfo->startTick = recvObj->conInfo->lastTick = ::GetTickCount();
	if (recvObj->startTick == 0)
		recvObj->startTick = recvObj->conInfo->startTick;

	if (recvObj->isDir == FALSE)
		recvObj->curFileInfo = *recvObj->fileInfo;
	recvObj->recvBuf = new char [pCfg->TransMax];

	// 0byte file 
	if (recvObj->isDir == FALSE && recvObj->curFileInfo.size == 0)
	{
		if (OpenRecvFile(recvObj))
		{
			CloseRecvFile(recvObj, TRUE);
			recvObj->status = FS_COMPLETE;
		}
		else recvObj->status = FS_ERROR;

		PostMessage(recvObj->hWndTcp, WM_TCPEVENT, recvObj->conInfo->sd, FD_CLOSE);
		return	TRUE;
	}

	DWORD	id;	
	recvObj->hThread = (HANDLE)~0;	// 
	// thread 
	if ((recvObj->hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvFileThread,
		recvObj, 0, &id)) == NULL)
	{
		EndRecvFile(recvObj);
		return	FALSE; 
	}
	if (recvObj->hWndTrans)	{
		PostMessage(recvObj->hWndTrans, WM_RECVOBJEVENT, (WPARAM)recvObj, (LPARAM)TRANS_INIT);
	}

	return	TRUE;
}

BOOL CShareMng::EndRecvFile(RecvFileObj* recvObj)
{
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	if (recvObj->hThread)
	{
		HANDLE	hThread = recvObj->hThread;
		recvObj->hThread = 0;
		WaitForSingleObject(hThread, INFINITE);
		::CloseHandle(hThread);
	}

	if (recvObj->status != FS_COMPLETE || recvObj->status != FS_CANCEL) recvObj->status = FS_ERROR;

	if (recvObj->hWndTrans)	{
		::SendMessage(recvObj->hWndTrans, WM_RECVOBJEVENT, (WPARAM)recvObj, (LPARAM)TRANS_DONE);
	}

	if (recvObj->conInfo)
	{	
		// 通知发送方取消该文件(夹)的传送。也许可以用中断连接的方式。
		char  szBuf[MAX_BUF] = {0};
		wsprintf(szBuf, "%x:%x", recvObj->packetNo, recvObj->fileInfo->id);
		pMsgMng->Send(recvObj->conInfo->addr, recvObj->conInfo->port, IM_RELEASEFILE, szBuf);
	}
// 	

//	int			target = CShareMng::GetFileInfoNo(recvObj->shareInfo, recvObj->fileInfo);
	FileInfo	*fileInfo = recvObj->fileInfo;
//	BOOL		isSingleTrans = recvObj->startTick == recvObj->conInfo->startTick;

	if (recvObj->conInfo) {
		::closesocket(recvObj->conInfo->sd);
		delete recvObj->conInfo;
		recvObj->conInfo = NULL;
	}	
	if (recvObj->recvBuf) delete [] recvObj->recvBuf;
	if (recvObj->fileInfo) delete recvObj->fileInfo;

// 	if (recvObj->status == FS_COMPLETE)
// 	{
// 		for (int cnt=0; cnt < recvObj->shareInfo->fileCnt; cnt++)
// 		{
// 			if (recvObj->shareInfo->fileInfo[cnt]->isSelected
// 				&& recvObj->shareInfo->fileInfo[cnt] != fileInfo)
// 			{
// 				FreeDecodeShareMsgFile(recvObj->shareInfo, target);
// 				return	SaveFile(recvObj);
// 			}
// 		}
// 	}
// 
// 	int ret = bManual ? IDCANCEL : RecvTransEndDlg(recvObj, this).Exec();
// 
// 	if (ret == EXEC_BUTTON || ret == FOLDER_BUTTON && recvObj->isDir && isSingleTrans)
// 	{
// 		char	buf[MAX_BUF];
// 		MakePath(buf, recvObj->saveDir, fileInfo->Fname());
// 		::ShellExecute(NULL, NULL, buf, 0, 0, SW_SHOW);
// 	}
// 	else if (ret == FOLDER_BUTTON)
// 		::ShellExecute(NULL, NULL, recvObj->saveDir, 0, 0, SW_SHOW);
// 
// 	if (ret == IDOK || ret == FOLDER_BUTTON || ret == EXEC_BUTTON)
// 		FreeDecodeShareMsgFile(shareInfo, target);
// 
// 	SetFileButton(this, FILE_BUTTON, shareInfo);
// 	//EvSize(SIZE_RESTORED, 0, 0);
// 
// 	if (ret == IDRETRY)
// 		PostMessage(WM_COMMAND, FILE_BUTTON, 0);

	return	TRUE;
}

BOOL CShareMng::OpenRecvFile(RecvFileObj* recvObj)
{
	char	path[MAX_BUF];

	if (MakePath(path, recvObj->isDir ? recvObj->path : recvObj->saveDir, recvObj->curFileInfo.Fname()) >= MAX_PATH)
		return	/*MessageBox(path, PATHTOOLONG_MSGSTR),*/ FALSE;
	if (IsSafePath(path, recvObj->curFileInfo.Fname()) == FALSE)
		return	/*MessageBox(path, NOTSAFEPATH_MSGSTR), */FALSE;

	if ((recvObj->hFile = ::CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
		return	recvObj->isDir ? FALSE : (/*MessageBox(CREATEFAIL_MSGSTR, path),*/ FALSE);

	if (recvObj->curFileInfo.attr & IM_FILE_RONLYOPT)
		::SetFileAttributes(path, FILE_ATTRIBUTE_READONLY);

	//::SetFilePointer(recvObj->hFile, OFFSET, 0, FILE_BEGIN);
	//::SetEndOfFile(recvObj->hFile);

	return	TRUE;
}

BOOL CShareMng::CloseRecvFile(RecvFileObj* recvObj, BOOL setAttr/* = FALSE*/)
{
	if (recvObj->hFile != INVALID_HANDLE_VALUE)
	{
		if (setAttr)
		{
			FILETIME	ft;
			UnixTime2FileTime(recvObj->curFileInfo.mtime, &ft);
#if 1	// protocol format
			if (recvObj->isDir || (recvObj->curFileInfo.mtime & 0xffffff00))
#endif
				::SetFileTime(recvObj->hFile, NULL, &ft, &ft);
		}
		recvObj->totalTrans += recvObj->offset;
		recvObj->totalFiles++;
		recvObj->offset = recvObj->woffset = 0;

		::CloseHandle(recvObj->hFile);
		recvObj->hFile = INVALID_HANDLE_VALUE;
	}
	return	TRUE;
}

BOOL CShareMng::DecodeDirEntry(FileInfo *info, char *buf)
{
	char	*tok, *ptr, *p;

	ConvertShareMsgEscape(buf);	// "::" -> ';'

	if ((tok = separate_token(buf, ':', &p)) == NULL)	// header size
		return	FALSE;

	if ((tok = separate_token(NULL, ':', &p)) == NULL)	// fname
		return	FALSE;
	info->SetFname(tok);
	//info->SetFnameExt(tok);
	while ((ptr = strchr(tok, '?')) != NULL)	// UNICODE 傑偱偺巄掕
		*ptr = '_';

	if (strlen(tok) >= MAX_PATH)
		return	FALSE;

	if ((tok = separate_token(NULL, ':', &p)) == NULL)	// size
		return	FALSE;
	info->size = hex2ll(tok);

	if ((tok = separate_token(NULL, ':', &p)) == NULL)	// attr
		return	FALSE;
	info->attr = strtoul(tok, 0, 16);

	while ((tok = separate_token(NULL, ':', &p)) != NULL)	// exattr
	{
		if ((ptr = strchr(tok, '=')) == NULL)
			continue;
		*ptr = 0;

		UINT	exattr = strtoul(tok, 0, 16);
		UINT	val = strtoul(ptr + 1, 0, 16);

		switch (exattr) {
		case IM_FILE_MTIME:		info->mtime = val; break;
		case IM_FILE_CREATETIME:	info->crtime = val; break;
		case IM_FILE_ATIME:		info->atime = val; break;	
		default: break;
		}
	}
	return	TRUE;
}

/*
int select(int nfds,  fd_set* readset,  fd_set* writeset,  
fe_set* exceptset,  struct timeval* timeout);
参数：
nfds： 需要检查的文件描述字个数(即检查到fd_set的第几位)，数值应该比三组fd_set中所含的最大fd值
		更大，一般设为三组fd_set中所含的最大fd值加1(如在readset, writeset, exceptset中所含最
		大的fd为5，则nfds＝6，因为fd是从0开始的 )。设这个值是为了提高效率，使函数不必检查
		fd_set的所有1024位。
readset： 用来检查可读性的一组文件描述字。
writeset： 用来检查可写性的一组文件描述字。	
exceptset： 用来检查是否有异常条件出现的文件描述字。(注：错误不包括在异常条件之内)
timeout： 有三种可能：
1.  timeout = NULL (阻塞：直到有一个fd位被置为1函数才返回)
2.  timeout所指向的结构设为非零时间(等待固定时间：有一个fd位被置为1或者时间耗尽，函数均返回)
3.  timeout所指向的结构，时间设为0(非阻塞：函数检查完每一个fd后立即返回)
返回值：返回对应位仍然为1的fd的总数。
Remark：
三组fd_set均将某些fd位置0，只有那些可读，可写以及有异常条件待处理的fd位仍然为1。
*/
DWORD CShareMng::RecvFileThread(void *obj)
{
	RecvFileObj* recvObj = (RecvFileObj*)obj;
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();
	fd_set		rfd;		//文件描述字(fd)的集合
	timeval		tv;
	int			sock_ret;
	BOOL		(CShareMng::*RecvFileFunc)(RecvFileObj *recvObj) =
					recvObj->isDir ? &CShareMng::RecvDirFile : &CShareMng::RecvFile;

	FD_ZERO(&rfd);
	FD_SET(recvObj->conInfo->sd, &rfd);

	for (int waitCnt=0; waitCnt < 180 && recvObj->hThread != NULL; waitCnt++)
	{
		tv.tv_sec = 1;		//seconds 
		tv.tv_usec = 0;		//microseconds
		if ((sock_ret = ::select(recvObj->conInfo->sd + 1,	//测试在规定的时间内套接字接口接收缓冲区是否有数据可读
			&rfd, NULL, NULL, &tv)) > 0)
		{
			waitCnt = 0;									//重置
			if ((pShareMng->*RecvFileFunc)(recvObj) != TRUE)
				break;
			if (recvObj->status == FS_COMPLETE)
				break;
		}
		else if (sock_ret == 0) 
		{
			FD_ZERO(&rfd);
			FD_SET(recvObj->conInfo->sd, &rfd); //FD_SET(0, &set); /*将set的第0位置1，如set原来是00000000，则现在变为100000000，这样fd==1的文件描述字就被加进set中了*/
			recvObj->conInfo->lastTick = ::GetTickCount();
			if (recvObj->hWndTrans)	{
				PostMessage(recvObj->hWndTrans, WM_RECVOBJEVENT, (WPARAM)recvObj, (LPARAM)TRANS_BUSY);
			}
		}
		else if (sock_ret == SOCKET_ERROR) {
			DUI__Trace(_T("RecvFileThread select failed, error code : %d\n"), WSAGetLastError());
			break;
		}
	}

	pShareMng->CloseRecvFile(recvObj, recvObj->status == FS_COMPLETE ? TRUE : FALSE);
	if (recvObj->status != FS_COMPLETE)
		recvObj->status = FS_ERROR;
	recvObj->conInfo->lastTick = ::GetTickCount();

	
	// 不能在线程里调用EndRecvFile，因为EndRecvFile中有等待线程退出的代码，会死锁。
	//pShareMng->EndRecvFile(recvObj, FALSE);
	// 通过通知传输数据窗口，由数据传输窗口去调用EndRecvFile
	PostMessage(recvObj->hWndTcp, WM_TCPEVENT, recvObj->conInfo->sd, FD_CLOSE);
	::ExitThread(0);
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
BOOL EncodeShareMsg(char *dest, ShareInfo *shareInfo, int size)
{
	ASSERT(dest && shareInfo);
	
	int		offset=0;
	char	fname[MAX_PATH];

	//*dest = 0;
	for (int cnt = 0; cnt < shareInfo->fileCnt; cnt++)
	{
		ForcePathToFname(fname, shareInfo->fileInfo[cnt]->fname);
		shareInfo->fileInfo[cnt]->id = cnt;
		// (shareInfo->fileInfo[cnt]->size >> 32) ? _T("%d:%s:%x%08x:%x:%s") : _T("%d:%s:%x%x:%x:"), 
		offset += wsprintf(dest + offset,
			(shareInfo->fileInfo[cnt]->size >> 32) ? _T("%d:%s:%x%08x:%x:") : _T("%d:%s:%x%x:%x:"), 
			cnt, fname, (int)((shareInfo->fileInfo[cnt]->size) >> 32), 
			shareInfo->fileInfo[cnt]->size, shareInfo->fileInfo[cnt]->mtime );
		offset += wsprintf(dest + offset, _T("%x:"), shareInfo->fileInfo[cnt]->attr);
		offset += wsprintf(dest + offset, _T("%x:"), shareInfo->fileInfo[cnt]->exattr);		// 附加属性 [12/15/2014 ybt]
		offset += wsprintf(dest + offset, _T("%c"), FILELIST_SEPARATOR);

		if (offset + MAX_BUF > size)
		{
			return FALSE;
		}
	}
	return	TRUE;
}

void ConvertShareMsgEscape(char *str)
{
	char	*ptr;
	while ((ptr = strstr(str, "::")) != NULL)
	{
		*ptr++ = ';';
		memmove(ptr, ptr + 1, strlen(ptr));
	}
}

ShareInfo* DecodeShareMsg(char *buf)
{
	ShareInfo	*shareInfo = new ShareInfo;
	FileInfo	*fileInfo = NULL;
	char		*tok, *p, *p2, *p3;
	char		*file = separate_token(buf, FILELIST_SEPARATOR, &p);

	for (int cnt=0; file; cnt++, file=separate_token(NULL, FILELIST_SEPARATOR, &p))
	{
		ConvertShareMsgEscape(file);	// "::" -> ';'
		if ((tok = separate_token(file, ':', &p2)) == NULL)
			break;
		fileInfo = new FileInfo(atoi(tok));

		if ((tok = separate_token(NULL, ':', &p2)) == NULL || strlen(tok) >= MAX_PATH)
			break;
		while ((p3 = strchr(tok, '?')) != NULL)	// UNICODE 傑偱偺巄掕
			*p3 = '_';
		fileInfo->SetFname(tok);
		//fileInfo->fname_ext = tok;

		if ((tok = separate_token(NULL, ':', &p2)) == NULL)
			break;
		fileInfo->size = hex2ll(tok);

		if ((tok = separate_token(NULL, ':', &p2)) == NULL)
			break;
		fileInfo->mtime = strtoul(tok, 0, 16);

		if ((tok = separate_token(NULL, ':', &p2)))
		{
			fileInfo->attr = strtoul(tok, 0, 16);
			u_int	attr_type = GET_MODE(fileInfo->attr);
			if (attr_type != IM_FILE_DIR && attr_type != IM_FILE_REGULAR)
			{
				delete fileInfo;
				continue;
			}
		}
		else fileInfo->attr = IM_FILE_REGULAR;

		// 附加属性 [12/15/2014 ybt] 
		if ((tok = separate_token(NULL, ':', &p2)))
		{
			fileInfo->exattr = strtoul(tok, 0, 16);
		}
		else fileInfo->exattr = 0;

		if ((shareInfo->fileCnt % FILE_BIG_ALLOC) == 0)
			shareInfo->fileInfo = (FileInfo **)realloc(shareInfo->fileInfo, (shareInfo->fileCnt + FILE_BIG_ALLOC) * sizeof(FileInfo *));

		shareInfo->fileInfo[shareInfo->fileCnt++] = fileInfo;
		fileInfo = NULL;
	}
	if (fileInfo)
		delete fileInfo;

	if (shareInfo->fileCnt <= 0)
	{
		delete shareInfo;
		return	NULL;
	}
	return	shareInfo;
}


BOOL FreeDecodeShareMsg(ShareInfo *info)
{
	while (info->fileCnt-- > 0)
		delete info->fileInfo[info->fileCnt];
	free(info->fileInfo);
	delete info;
	return	TRUE;
}

BOOL FreeDecodeShareMsgFile(ShareInfo *info, int index)
{
	if (index >= info->fileCnt)
		return	FALSE;
	delete info->fileInfo[index];
	memmove(info->fileInfo + index, info->fileInfo + index +1, sizeof(FileInfo *) * (--info->fileCnt - index));
	return	TRUE;
}

BOOL FreeDecodeShareMsgFile(ShareInfo *info, FileInfo* fileInfo)
{
	for (int idx = 0; idx < info->fileCnt; idx++)
	{
		if (info->fileInfo[idx] == fileInfo)
		{
			return FreeDecodeShareMsgFile(info, idx);
		}
	}
	return FALSE;
}