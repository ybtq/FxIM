//////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "StdAfx.h"
#include "Utils/BaseDef.h"
#include "Utils/BLOWFISH.H"



//////////////////////////////////////////////////////////////////////////
//struct PubKey
//////////////////////////////////////////////////////////////////////////
PubKey::PubKey()
{
	key = NULL; 
	capa = 0;
	keyLen = 0;
	e = 0;
}

PubKey::~PubKey()
{
	ReSet();
}

PubKey &PubKey::operator = (PubKey &pubKey)
{
	capa = pubKey.capa;
	e = pubKey.e;
	key = new BYTE[keyLen = pubKey.keyLen];
	memcpy(key, pubKey.key, keyLen);

	return *this;
}

int  PubKey::KeyBlobLen()
{
	return keyLen + 8 + 12; /* PUBLICKEYSTRUC + RSAPUBKEY */
}

void PubKey::Set(BYTE *_key, int _keyLen, int _e, int _capa)
{
	ASSERT(_key);

	ReSet(); 
	e = _e; 
	capa = _capa;
	key = new BYTE [keyLen = _keyLen];
	memcpy(key, _key, keyLen);
}

void PubKey::ReSet()
{
	delete [] key; 
	key = NULL; 
	capa = 0;
	e = 0;
}

BOOL PubKey::KeyBlob(BYTE *blob, int maxLen, int *len)
{
	ASSERT(blob && len);

	if ((*len = KeyBlobLen()) > maxLen)
		return FALSE;

	/* PUBLICSTRUC */
	blob[0] = PUBLICKEYBLOB; 
	blob[1] = CUR_BLOB_VERSION;	
	*(WORD *)(blob +2) = 0;				//第3和第4个字单位为空
	*(ALG_ID *)(blob +4) = CALG_RSA_KEYX;	//CALG_RSA_KEYX = 1110010000000000 占用2个BYTE
	
	/* RSAPUBKEY */	
	memcpy(blob +8, "RSA1", 4);	
	*(DWORD *)(blob +12) = keyLen * 8;
	*(int *)(blob +16) = e;
	
	/* PUBKEY_DATA */
	memcpy(blob +20, key, keyLen);

	/*odprintf(_T("=> PubKey::KeyBlob中blob长度为%d，以十六进制显示为:\n"), *len);
	for (int i = 0; i < *len; i++)
	{
		odprintf(_T("%X"), blob[i]);
	}
	odprintf(_T("\n"));*/
	
	return	TRUE;
}

void PubKey::SetByBlob(BYTE *blob, int _capa)
{
	ASSERT(blob);

	ReSet();
	
	keyLen = *(int *)(blob + 12) / 8;
	key = new BYTE [keyLen];
	memcpy(key, blob + 20, keyLen);
	e = *(int *)(blob + 16);
	capa = _capa;
}

//////////////////////////////////////////////////////////////////////////
//struct AddrObj
//////////////////////////////////////////////////////////////////////////
AddrObj::AddrObj()
{
	addr = ~0x0L;
	portNo = 0;
}

AddrObj::AddrObj(ULONG _addr, int _portNo)
{
	addr = _addr;
	portNo = _portNo;
}

//////////////////////////////////////////////////////////////////////////
//struct HostSub
//////////////////////////////////////////////////////////////////////////

HostSub &HostSub::operator = (HostSub &hostSub)
{
	this->addr	= hostSub.addr;
	this->portNo= hostSub.portNo;
	lstrcpy(this->hostName, hostSub.hostName);
//	lstrcpy(this->mac, hostSub.mac);	
	lstrcpy(this->userName, hostSub.userName);

	return *this;
}

BOOL HostSub::operator == (HostSub &hostSub)
{
	if (addr == hostSub.addr
		&& strcmp(hostName, hostSub.hostName) == 0
//		&& strcmp(mac, hostSub.mac) == 0
		&& portNo == hostSub.portNo
		&& strcmp(userName, hostSub.userName) == 0)
	{
		return TRUE;
	}	
	return FALSE;
}

BOOL HostSub::IsSameHostSub(HostSub *hostSub)
{
	ASSERT(hostSub);

	BOOL status = FALSE;
	status = (strcmp(this->hostName, hostSub->hostName) == 0);
	status |= (this->addr == hostSub->addr);
	return status;
}


//////////////////////////////////////////////////////////////////////////
//struct HostSet
//////////////////////////////////////////////////////////////////////////
HostSet::HostSet()
{
	addr = 0;	
	*nickNameEx = 0;
	*hostName = 0;
	status = 0;
	strncpy(groupNameEx, DEFAULTGROUPNAMEEX_STR, MAX_NAMEBUF); //初始化备注组名为“我的好友”
}

HostSet::~HostSet()
{
}

//////////////////////////////////////////////////////////////////////////
//struct Host
//////////////////////////////////////////////////////////////////////////
Host::Host()
{
	hostSet = NULL;
	*nickName	= 0;
	*groupName	= 0;
	*avatar = 0;
	refCnt = 0;
	tag = 0;
	hIconAvatar = NULL;
}

Host::~Host()
{
	if (hIconAvatar != NULL) {
		::DeleteObject(hIconAvatar);
		hIconAvatar = NULL;
	}
	refCnt = 0;
}

void Host::Init(HostSub *hostSub, ULONG command, time_t update_time, char *nickName, char *groupName)
{
	ASSERT(hostSub);
	
	this->hostStatus = GET_OPT(command);
	this->hostSub = *hostSub;
	this->updateTime = update_time;
	lstrcpy(this->nickName, nickName);
	lstrcpy(this->groupName, groupName);
}



/*
	host array class
*/
THosts::THosts(void)
{
	hostCnt = 0;
	memset(array, 0, sizeof(array));
	for (int kind=0; kind < MAX_ARRAY; kind++)
		enable[kind] = FALSE;
}

THosts::~THosts()
{
	for (int kind=0; kind < MAX_ARRAY; kind++) {
		if (array[kind])
			free(array[kind]);
	}
}


int THosts::Cmp(HostSub *hostSub1, HostSub *hostSub2, Kind kind)
{
	switch (kind) 
	{
	case NAME: 
	case NAME_ADDR:
		{
			int	cmp;
			if (cmp = stricmp(hostSub1->userName, hostSub2->userName))
				return	cmp;
			if ((cmp = stricmp(hostSub1->hostName, hostSub2->hostName)) || kind == NAME)
				return	cmp;
		}	// if cmp == 0 && kind == NAME_ADDR, through...
	case ADDR:
		if (hostSub1->addr > hostSub2->addr)
			return	1;
		if (hostSub1->addr < hostSub2->addr)
			return	-1;
		if (hostSub1->portNo > hostSub2->portNo)
			return	1;
		if (hostSub1->portNo < hostSub2->portNo)
			return	-1;
		return	0;
	}
	return	-1;
}

Host *THosts::Search(Kind kind, HostSub *hostSub, int *insertIndex)
{
	int	min = 0, max = hostCnt -1, cmp, tmpIndex;

	if (insertIndex == NULL)
		insertIndex = &tmpIndex;

	while (min <= max)
	{
		*insertIndex = (min + max) / 2;

		if ((cmp = Cmp(hostSub, &array[kind][*insertIndex]->hostSub, kind)) == 0)
			return	array[kind][*insertIndex];
		else if (cmp > 0)
			min = *insertIndex +1;
		else
			max = *insertIndex -1;
	}

	*insertIndex = min;
	return	NULL;
}

BOOL THosts::AddHost(Host *host)
{
	int		kind;
	int		insertIndex[MAX_ARRAY];

	for (kind=0; kind < MAX_ARRAY; kind++) {
		if (enable[kind] == FALSE)
			continue;
		if (Search((Kind)kind, &host->hostSub, &insertIndex[kind]) != NULL)
			return	FALSE;
	}

#define HOST_BIG_ALLOC	100
	for (kind=0; kind < MAX_ARRAY; kind++) {
		if (enable[kind] == FALSE)
			continue;
		if ((hostCnt % HOST_BIG_ALLOC) == 0)
		{
			Host	**tmpArray = (Host **)realloc(array[kind], (hostCnt + HOST_BIG_ALLOC) * sizeof(Host *));
			if (tmpArray == NULL)
				return	FALSE;
			array[kind] = tmpArray;
		}
		memmove(array[kind] + insertIndex[kind] + 1, array[kind] + insertIndex[kind], (hostCnt - insertIndex[kind]) * sizeof(Host *));
		array[kind][insertIndex[kind]] = host;
	}
	host->RefCnt(1);
	hostCnt++;
	return	TRUE;
}

BOOL THosts::DelHost(Host *host)
{
	int		kind;
	int		insertIndex[MAX_ARRAY];

	for (kind=0; kind < MAX_ARRAY; kind++)
	{
		if (enable[kind] == FALSE)
			continue;
		if (Search((Kind)kind, &host->hostSub, &insertIndex[kind]) == NULL)
			return	FALSE;
	}

	hostCnt--;

	for (kind=0; kind < MAX_ARRAY; kind++) 
	{
		if (enable[kind] == FALSE)
			continue;
		memmove(array[kind] + insertIndex[kind], array[kind] + insertIndex[kind] + 1, 
			(hostCnt - insertIndex[kind]) * sizeof(Host *));
	}
	host->RefCnt(-1);

	return	TRUE;
}

BOOL THosts::PriorityHostCnt(int priority, int range)
{
	int		member = 0;

	for (int cnt=0; cnt < hostCnt; cnt++)
		if (array[NAME][cnt]->priority >= priority && array[NAME][cnt]->priority < priority + range)
			member++;
	return	member;
}

//////////////////////////////////////////////////////////////////////////
//struct MsgBuf
//////////////////////////////////////////////////////////////////////////
MsgBuf::MsgBuf()
{
	command	= 0L;
	dummy	= 0;
	exOffset= 0;
	*msgBuf	= 0;
	packetNo= 0;
	portNo	= 0;
	version	= 0;
	recvTime = time(NULL);
}
void MsgBuf::Init(MsgBuf *org)
{
	if (org == NULL) 
	{
		memset(this, 0, (char *)&this->dummy - (char *)this);
		*msgBuf = 0;
		return;
	}
	memcpy(this, org, (char *)&this->dummy - (char *)this);
	lstrcpy(this->msgBuf, org->msgBuf);
	lstrcpy(this->msgBuf + exOffset, org->msgBuf + exOffset);
}


//////////////////////////////////////////////////////////////////////////
//struct ShareInfo
//////////////////////////////////////////////////////////////////////////
ShareInfo::ShareInfo(ULONG _packetNo /* = 0 */)
{
	packetNo	= _packetNo;
	host		= NULL;
	transStat	= NULL;
	fileInfo	= NULL;
	hostCnt		= 0;
	fileCnt		= 0;
}

void ShareInfo::LinkList(ShareInfo *top)
{
	prior = top->prior;
	next = top;
	top->prior->next = this;
	top->prior = this;
}

/*
	TList class
*/
TList::TList(void)
{
	top.prior = top.next = &top;
}

void TList::AddObj(TListObj * obj)
{
	obj->prior = top.prior;
	top.prior->next = obj;
	obj->next = &top;
	top.prior = obj;
}

void TList::DelObj(TListObj * obj)
{
	if (obj->next)
		obj->next->prior = obj->prior;
	if (obj->prior)
		obj->prior->next = obj->next;
	obj->next = obj->prior = NULL;
}

TListObj* TList::TopObj(void)
{
	return	top.next == &top ? NULL : top.next;
}

TListObj* TList::NextObj(TListObj *obj)
{
	return	obj->next == &top ? NULL : obj->next;
}


void TBroadcastList::Reset()
{
	TBroadcastObj	*obj;

	while ((obj = Top()) != NULL)
	{
		DelObj(obj);
		delete obj;
	}
}




//////////////////////////////////////////////////////////////////////////
//全局函数
//////////////////////////////////////////////////////////////////////////
//本函数来自《Visual C++网络通信程序开发指南》第二章 
//功能：将错误信息代码转换为文字，并显示
//在Release编译下，运行于未装VC的计算机，此函数仍有效  2010-11-05
void ShowErrorInfo(DWORD errorCode, LPCTSTR exMsg /* = NULL */)
{
	char buf[MAX_PATH];	
	void *info;
	
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER| FORMAT_MESSAGE_FROM_SYSTEM,
		0, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&info, 0, NULL);

	wsprintf(buf, _T("%s Error: %s"), exMsg, (char*)info);
	::MessageBox(NULL, buf, MSGCAPTION_STR, MB_ICONERROR| MB_OK);	

	::LocalFree(info);
}


ULONG ResolveAddr(const char *_host)
{
	if (_host == NULL)
		return 0;

	ULONG	addr = ::inet_addr(_host);

	if (addr == 0xffffffff)
	{
		hostent	*ent = ::gethostbyname(_host);
		addr = ent ? *(ULONG *)ent->h_addr_list[0] : 0;
	}

	return	addr;
}


void MakeHash(const BYTE *data, int len, char *hashStr)
{
	CBlowFish	bl((BYTE *)"im", 2);
	BYTE		*buf = new BYTE [len + 8];

	bl.Encrypt(data, buf, len);
	bin2hexstr(buf + len - 8, 8, hashStr);
	delete [] buf;
}

inline u_char hexchar2char(u_char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	ch = toupper(ch);
	if (ch >= 'A' && ch <= 'Z')
		return ch - 'A' + 10;
	return 0;
}

BOOL hexstr2bin(const char *buf, BYTE *bindata, int maxlen, int *len)
{
	for (*len=0; buf[0] && buf[1] && *len < maxlen; buf+=2, (*len)++)
	{
		bindata[*len] = hexchar2char(buf[0]) << 4 | hexchar2char(buf[1]);
	}
	return	TRUE;
}

/* hexstr to little-endian binary */
BOOL hexstr2bin_bigendian(const char *buf, BYTE *bindata, int maxlen, int *len)
{
	*len = 0;
	for (int buflen = strlen(buf); buflen >= 2 && *len < maxlen; buflen-=2, (*len)++)
	{
		bindata[*len] = hexchar2char(buf[buflen-1]) | hexchar2char(buf[buflen-2]) << 4;
	}
	return	TRUE;
}

static char *hexstr = "0123456789abcdef";
void bin2hexstr(const BYTE *bindata, int len, char *buf)
{
	for (int cnt=0; cnt < len; cnt++)
	{
		*buf++ = hexstr[bindata[cnt] >> 4];
		*buf++ = hexstr[bindata[cnt] & 0x0f];
	}
	*buf = 0;
}

/* little-endian binary to hexstr */
void bin2hexstr_bigendian(const BYTE *bindata, int len, char *buf)
{
	while (len-- > 0)
	{
		*buf++ = hexstr[bindata[len] >> 4];
		*buf++ = hexstr[bindata[len] & 0x0f];
	}
	*buf = 0;
}


char *separate_token(char *buf, char separetor, char **handle)
{
	char *_handle;

	if (handle == NULL)
		handle = &_handle;

	if (buf)
		*handle = buf;

	if (*handle == NULL || **handle == 0)
		return	NULL;

	while (**handle == separetor)
		(*handle)++;
	buf = *handle;

	if (**handle == 0)
		return	NULL;

	while (**handle && **handle != separetor)
		(*handle)++;

	if (**handle == separetor)
		*(*handle)++ = 0;

	return	buf;
}


//来自IPMSG的hex2ll函数
//功能：十六进制的字符转换成long long (即_int64)值
_int64 hex2ll(char *src)
{
	ASSERT(src);

	_int64 ret = 0;
	for (; *src; src++)
	{
		if (*src >= '0' && *src <= '9')	{
			ret = (ret << 4) | (*src - '0');
		}
		else if (toupper(*src) >= 'A' && toupper(*src) <= 'F'){
			ret = (ret << 4) | (toupper(*src) - 'A' + 10);
		}
		else {
			continue;
		}
	}
	return ret;
}



//#endif

void ForcePathToFname(char *dest, const char *src)
{
	if (PathToFname(dest, src))
		return;

	if (src[1] == ':')
		wsprintf(dest, _T("(%c-drive)"), *src);
	else if (src[0] == '\\' && src[1] == '\\')
	{
		if (PathToFname(dest, src + 2) == FALSE)
			wsprintf(dest, _T("(root)"));
	}else{
		wsprintf(dest, _T("(unknown)"));
	}
}

BOOL PathToFname(char *dest, const char *src)
{
	char	path[MAX_PATH], *fname=NULL;

	if (::GetFullPathName(src, sizeof(path), path, &fname) == 0 || fname == NULL)
		return	FALSE;

	strncpy(dest, fname, MAX_PATH);

	return	TRUE;
}


BOOL PathToDir(char *dest, const char *src)
{
	char	path[MAX_BUF], *fname=NULL;

	if (::GetFullPathName(src, sizeof(path), path, &fname) == 0 || fname == NULL)
		return	strncpy(dest, src, MAX_PATH), FALSE;

	if (fname - path > 3 || path[1] != ':')
		*(fname - 1) = 0;
	else
		*fname = 0;	

	strncpy(dest, path, MAX_PATH);

	return	TRUE;
}

int MakePath(char *dest, const char *dir, const char *file)
{
	ASSERT(dest && dir && file);
	
	BOOL	separetor = TRUE;
	int		len;

	if ((len = strlen(dir)) == 0)
		return	wsprintf(dest, "%s", file);

	if (dir[len -1] == '\\')
	{
		if (len >= 2 && IsDBCSLeadByte(dir[len -2]) == FALSE)
		{
			separetor = FALSE;
		}
		else 
		{
			u_char *p = NULL;
			for (p=(u_char *)dir; *p && p[1]; IsDBCSLeadByte(*p) ? p+=2 : p++)
				;
			if (*p == '\\')
				separetor = FALSE;
		}
	}
	return	wsprintf(dest, "%s%s%s", dir, separetor ? "\\" : "", file);
}

BOOL IsSafePath(const char *fullPath, const char *fname)
{
	char	fname2[MAX_PATH];

	if (PathToFname(fname2, fullPath) == FALSE)
		return	FALSE;

	return	strcmp(fname, fname2) == 0 ? TRUE : FALSE;
}

_int64 GetFolderSize(const char *fname)
{
	_int64			fsize = 0;
	HANDLE			hFind = NULL;
	char			fileFilter[MAX_PATH] = {0};
	char			subFolder[MAX_PATH] = {0};
	WIN32_FIND_DATA	fdat;

	wsprintf(fileFilter, _T("%s\\*.*"), fname);
	hFind = ::FindFirstFile(fileFilter, &fdat);
	do {
		if (fdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(fdat.cFileName, ".") != 0		//如果不是当前或上级目录
				&& strcmp(fdat.cFileName, "..") != 0)
			{
				wsprintf(subFolder, _T("%s\\%s"), fname, fdat.cFileName);
				fsize += GetFolderSize(subFolder);
			}
		}
		else 
		{
			fsize += ((_int64)fdat.nFileSizeHigh << 32 | fdat.nFileSizeLow);
		}
	} while(::FindNextFile(hFind, &fdat));

	::FindClose(hFind);

	return fsize;	
}

BOOL GetFileInfomation(const char *path, WIN32_FIND_DATA *fdata)
{
	HANDLE	fh;

	if ((fh = ::FindFirstFile(path, fdata)) != INVALID_HANDLE_VALUE)
	{
		::FindClose(fh);
		return	TRUE;
	}

	memset(fdata, 0, sizeof(WIN32_FIND_DATA));

	if ((fh = ::CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION	info;
		BOOL	info_ret = ::GetFileInformationByHandle(fh, &info);
		::CloseHandle(fh);
		if (info_ret)
			return	memcpy(fdata, &info, (char *)&info.dwVolumeSerialNumber - (char *)&info), TRUE;
	}

	return	(fdata->dwFileAttributes = ::GetFileAttributes(path)) == 0xffffffff ? FALSE : TRUE;
}


#define ONEGB	1073741824			//1024 * 1024 * 1024
#define ONEMB	1048576				//1024 * 1024
#define ONEKB	1024
int MakeSizeString(LPTSTR dest, _int64 size)
{
	if (size >= ONEGB){
		return wsprintf(dest, _T("%d.%d GB"), (int)(size / ONEGB), 
			(int)((size * 10 / ONEGB) % 10));
	}
	else if (size >= ONEMB){
		return wsprintf(dest, _T("%d.%d MB"), (int)(size / ONEMB),
			(int)((size * 10 / ONEMB) % 10));
	}
	else /*if (size >= ONEKB)*/ {
		return wsprintf(dest, _T("%d.%d KB"), (int)(size / ONEKB), 
			(int)((size * 10 / ONEKB) % 10));
	}
// 	else {
// 		return wsprintf(dest, _T("%d Byte"), size);
// 	}
}

int MakeTransRateStr(char *dest, DWORD ticks, _int64 cur_size, _int64 total_size)
{
	ASSERT(dest);

	int len = 0;
	int	percent = 0;

	len += MakeSizeString(dest + len, cur_size);								//已传输
	percent	= (int)(cur_size * 100 / (total_size ? total_size : 1));		//百分比
	len += wsprintf(dest + len, _T("(%d%%)"), percent);

	dest[len++] = ' ';
	len += MakeSizeString(dest + len, cur_size * 1000 / (ticks ? ticks : 10));	//速度
	strcat(dest, _T("/S"));

	return percent;			//返回传输百分比
}

// 飞鸽传过来的文件夹数据 并没有文件个数及文件夹大小的信息
void MakeDirTransRateStr(char *buf, DWORD ticks, _int64 cur_size, int files)
{
	int len = 0;
	buf[len++] = ' ';
	len += wsprintf(buf + len, "Total ");
	len += MakeSizeString(buf + len, cur_size);
	len += wsprintf(buf + len, "/%dfiles (", files);
	len += MakeSizeString(buf + len, cur_size * 1000 / (ticks ? ticks : 1));
	len += wsprintf(buf + len, "/s)" );
}
