#pragma  once

class CShareMng 
{
private:
	CShareMng();
	~CShareMng();
	DECLARE_SINGLETON_CLASS(CShareMng)
public:
	enum			{ TRANS_INIT, TRANS_BUSY, TRANS_DONE };
protected:
	TList			m_listShareInfo;
	TList			m_listSendFileObj;
	TList			m_listRecvFileObj;
	TList			m_listConInfo;

protected:
	BOOL			SetFileInfo(const char *fname, FileInfo *info, UINT exattr = 0);


public:
	ShareInfo		*CreateShare(int packetNo);					// for send shareinfo.
	ShareInfo		*Search(int packetNo);
	void			DestroyShare(ShareInfo *info);
	BOOL			AddFileShare(ShareInfo *info, const char *fname, UINT exattr = 0);
	BOOL			DelFileShare(ShareInfo *info, int fileNo);
	BOOL			EndHostShare(int packetNo, HostSub *hostSub, FileInfo *fileInfo=NULL, BOOL done=TRUE);
	BOOL			AddHostShare(ShareInfo *info, SendEntry *entry, int entryNum);

//	BOOL			GetShareCntInfo(ShareCntInfo *info, ShareInfo *shareInfo=NULL);
	BOOL			GetAcceptableFileInfo(ConnectInfo *info, char *buf, AcceptFileInfo *fileInfo);
	static int		GetFileInfoNo(ShareInfo *info, FileInfo *fileInfo);

	// ConnectInfo
	BOOL			AddConnectInfo(ConnectInfo* conInfo);
	BOOL			RemoveConnectInfo(ConnectInfo* conInfo);
	ConnectInfo*	GetConnectInfo(SOCKET sd);


	// File Send.
	BOOL			AddSendFileObjs(ShareInfo* shareInfo, HWND hWndTrans);
	SendFileObj*	GetSendFileObj(FileInfo *fileInfo);
	SendFileObj*	GetSendFileObj(SOCKET sd);
	SendFileObj*	TopSendObj() { return (SendFileObj*)m_listSendFileObj.TopObj(); };
	SendFileObj*	NextSendObj(SendFileObj* obj) { return (SendFileObj*)m_listSendFileObj.NextObj(obj); }

	BOOL			StartSendFile(SOCKET sd, HWND hWnd);

	BOOL			OpenSendFile(SendFileObj *sendObj, const char *fname);	
	BOOL			EndSendFile(SendFileObj* sendObj);
	BOOL			CloseSendFile(SendFileObj* sendObj);
	int				MakeDirHeader(SendFileObj *sendObj, BOOL find);
	BOOL			SendFile(SendFileObj* sendObj);
	BOOL			SendDirFile(SendFileObj *sendObj);	
	static DWORD	SendFileThread(void *obj);

	// File Recv.
	BOOL			AddRecvFileObjs(ShareInfo* shareInfo, HWND hWndTcp, 
						HWND hWndTrans, ULONG addr, USHORT port);
	RecvFileObj*	GetRecvFileObj(SOCKET sd);
	RecvFileObj*	TopRecvObj() { return (RecvFileObj*)m_listRecvFileObj.TopObj(); }
	RecvFileObj*	NextRecvObj(RecvFileObj* obj) { return (RecvFileObj*)m_listRecvFileObj.NextObj(obj); }
	void			SaveRecvFileObjs(ULONG nPacketNo, LPCTSTR szSaveDir);
	BOOL			RecvFile(RecvFileObj* recvObj);
	BOOL			RecvDirFile(RecvFileObj* recvObj);
	BOOL			SaveFile(RecvFileObj* recvObj, BOOL bFailIfExists = FALSE);
	BOOL			ConnectRecvFile(RecvFileObj* recvObj);
	BOOL			StartRecvFile(RecvFileObj* recvObj);
	BOOL			EndRecvFile(RecvFileObj* recvObj);
	BOOL			OpenRecvFile(RecvFileObj* recvObj);
	BOOL			CloseRecvFile(RecvFileObj* recvObj, BOOL setAttr = FALSE);
	BOOL			DecodeDirEntry(FileInfo *fileInfo, char *buf);
	static DWORD	RecvFileThread(void *obj);
};

BOOL		EncodeShareMsg(char *dest, ShareInfo *shareInfo, int size);
ShareInfo*	DecodeShareMsg(char *buf);
BOOL		FreeDecodeShareMsg(ShareInfo *info);
BOOL		FreeDecodeShareMsgFile(ShareInfo *info, int index);
BOOL		FreeDecodeShareMsgFile(ShareInfo *info, FileInfo* fileInfo);			// add [12/10/2014 ybt]
void		ConvertShareMsgEscape(char *str);