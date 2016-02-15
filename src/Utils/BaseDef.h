#ifndef _BASE_DEF_H_
#define _BASE_DEF_H_
#include "StdAfx.h"
//命令字为32位无符号整数，包含命令（低8位）和选项（高24位）
//带有BR标识的为广播命令
//所有命令和选项开头都以IM_取代IPMSG_
/*  macro  */
#define GET_MODE(command)	(command & 0x000000ffUL)
#define GET_OPT(command)	(command & 0xffffff00UL)

/*  header  */
#define IM_VERSION				0x0001
#define IM_DEFAULT_PORT			0x0979    //2425

/*  command  */
#define IM_NOOPERATION			0x00000000UL

#define IM_BR_ENTRY				0x00000001UL
#define IM_BR_EXIT				0x00000002UL
#define IM_ANSENTRY				0x00000003UL
#define IM_BR_ABSENCE			0x00000004UL

#define IM_BR_ISGETLIST			0x00000010UL
#define IM_OKGETLIST			0x00000011UL
#define IM_GETLIST				0x00000012UL
#define IM_ANSLIST				0x00000013UL
#define IM_BR_ISGETLIST2		0x00000018UL

#define IM_SENDMSG				0x00000020UL
#define IM_RECVMSG				0x00000021UL
#define IM_READMSG				0x00000030UL
#define IM_DELMSG				0x00000031UL
#define IM_ANSREADMSG			0x00000032UL

#define IM_GETINFO				0x00000040UL
#define IM_SENDINFO				0x00000041UL

#define IM_GETABSENCEINFO		0x00000050UL
#define IM_SENDABSENCEINFO		0x00000051UL

#define IM_GETFILEDATA			0x00000060UL
#define IM_RELEASEFILES			0x00000061UL
#define IM_GETDIRFILES			0x00000062UL
#define IM_RELEASEFILE			0x00000063UL		// 接收方取消传送某个FileInfo [12/15/2014 ybt]

#define IM_GETPUBKEY			0x00000072UL
#define IM_ANSPUBKEY			0x00000073UL

#define IM_GETAVATAR			0x00000080UL		// 请求对方头像 [12/15/2014 ybt]
#define IM_ANSAVATAR			0x00000081UL		// 回答请求头像 [12/15/2014 ybt]

#define IM_SENDMSGIMAGE			0x00000085UL		// 发送消息中的图片

/*  option for all command  */
#define IM_ABSENCEOPT			0x00000100UL
#define IM_SERVEROPT			0x00000200UL
#define IM_DIALUPOPT			0x00010000UL
#define IM_FILEATTACHOPT		0x00200000UL
#define IM_ENCRYPTOPT			0x00400000UL
#define IM_UTF8OPT				0x00800000UL		// IPMSG v2.11 add
#define IM_FXIMOPT				0x10000000UL		// FxIM or not. [11/28/2014 ybt]
#define IM_GETAVATARFILE		0x20000000UL		// 请求对方发送头像文件


/*  option for send command  */
#define IM_SENDCHECKOPT			0x00000100UL
#define IM_SECRETOPT			0x00000200UL
#define IM_BROADCASTOPT			0x00000400UL
#define IM_MULTICASTOPT			0x00000800UL
#define IM_NOPOPUPOPT			0x00001000UL
#define IM_AUTORETOPT			0x00002000UL
#define IM_RETRYOPT				0x00004000UL
#define IM_PASSWORDOPT			0x00008000UL
#define IM_NOLOGOPT				0x00020000UL
#define IM_NEWMUTIOPT			0x00040000UL
#define IM_NOADDLISTOPT			0x00080000UL
#define IM_READCHECKOPT			0x00100000UL
//#define IM_SENDRTFOPT			0x02000000UL			//新添加，发送RTF格式文档(2011-02-20)
//#define IM_SENDFINISH			0x04000000UL			//新添加，表示单个信息已发送完全
//#define IM_MYSELF				0x08000000UL			//新添加，表示为添加到RichEdit的数据是自己本人的
#define IM_SECRETEXOPT			(IM_READCHECKOPT|IM_SECRETOPT)

/* encryption flags for encrypt command */   //算法
#define IM_RSA_512				0x00000001UL
#define IM_RSA_1024				0x00000002UL
#define IM_RSA_2048				0x00000004UL
#define IM_RC2_40				0x00001000UL
#define IM_RC2_128				0x00004000UL
#define IM_RC2_256				0x00008000UL
#define IM_BLOWFISH_128			0x00020000UL
#define IM_BLOWFISH_256			0x00040000UL
#define IM_SIGN_MD5				0x10000000UL

/* compatibilty for Win beta version */
#define IM_RC2_40OLD			0x00000010UL	// for beta1-4 only
#define IM_RC2_128OLD			0x00000040UL	// for beta1-4 only
#define IM_BLOWFISH_128OLD		0x00000400UL	// for beta1-4 only
#define IM_RC2_40ALL			(IM_RC2_40|IM_RC2_40OLD)
#define IM_RC2_128ALL			(IM_RC2_128|IM_RC2_128OLD)
#define IM_BLOWFISH_128ALL		(IM_BLOWFISH_128|IM_BLOWFISH_128OLD)

/* file types for fileattach command */   //文件传输
#define IM_FILE_REGULAR			0x00000001UL
#define IM_FILE_DIR				0x00000002UL
#define IM_FILE_RETPARENT		0x00000003UL	// return parent directory
#define IM_FILE_SYMLINK			0x00000004UL
#define IM_FILE_CDEV			0x00000005UL	// for UNIX
#define IM_FILE_BDEV			0x00000006UL	// for UNIX
#define IM_FILE_FIFO			0x00000007UL	// for UNIX
#define IM_FILE_RESFORK			0x00000010UL	// for Mac

/* file attribute options for fileattach command */
#define IM_FILE_RONLYOPT		0x00000100UL
#define IM_FILE_HIDDENOPT		0x00001000UL
#define IM_FILE_EXHIDDENOPT		0x00002000UL	// for MacOS X
#define IM_FILE_ARCHIVEOPT		0x00004000UL
#define IM_FILE_SYSTEMOPT		0x00008000UL


/* extend attribute types for fileattach command */
#define IM_FILE_UID				0x00000001UL
#define IM_FILE_USERNAME		0x00000002UL	// uid by string
#define IM_FILE_GID				0x00000003UL
#define IM_FILE_GROUPNAME		0x00000004UL	// gid by string
#define IM_FILE_PERM			0x00000010UL	// for UNIX
#define IM_FILE_MAJORNO			0x00000011UL	// for UNIX devfile
#define IM_FILE_MINORNO			0x00000012UL	// for UNIX devfile
#define IM_FILE_CTIME			0x00000013UL	// for UNIX
#define IM_FILE_MTIME			0x00000014UL
#define IM_FILE_ATIME			0x00000015UL
#define IM_FILE_CREATETIME		0x00000016UL
#define IM_FILE_CREATOR			0x00000020UL	// for Mac
#define IM_FILE_FILETYPE		0x00000021UL	// for Mac
#define IM_FILE_FINDERINFO		0x00000022UL	// for Mac
#define IM_FILE_ACL				0x00000030UL
#define IM_FILE_ALIASFNAME		0x00000040UL	// alias fname
#define IM_FILE_UNICODEFNAME	0x00000041UL	// UNICODE fname

#define IM_FILE_AVATAR			0x00000050UL	// For FxIM 文件是个头像 [12/10/2014 ybt] 
#define IM_FILE_CHATIMAGE		0x00000051UL	// For FxIM 文件是聊天的图片 [12/15/2014 ybt] 

#define FILELIST_SEPARATOR	'\a'
#define HOSTLIST_SEPARATOR	'\a'
#define HOSTLIST_DUMMY		"\b"

/*  end of IP Messenger Communication Protocol version 1.2 define  */


/*  IP Messenger for Windows  internal define  */
#define IM_REVERSEICON			0x0100
#define IM_TIMERINTERVAL		500
#define IM_ENTRYMINSEC			5
#define IM_GETLIST_FINISH		0

#define IM_BROADCAST_TIMER		0x0101
#define IM_SEND_TIMER			0x0102
#define IM_LISTGET_TIMER		0x0104
#define IM_LISTGETRETRY_TIMER	0x0105
#define IM_ENTRY_TIMER			0x0106
#define IM_DUMMY_TIMER			0x0107
#define IM_RECV_TIMER			0x0108
#define IM_ANS_TIMER			0x0109
#define IM_MSGJUMP_TIMER		0x0120				// 消息闪动
#define IM_FLASHWINDOW_TIMER	0X0121				// 聊天窗口闪动

#define IM_NICKNAME			1
#define IM_FULLNAME			2

#define IM_NAMESORT			0x00000000
#define IM_IPADDRSORT		0x00000001
#define IM_HOSTSORT			0x00000002
#define IM_NOGROUPSORTOPT	0x00000100
#define IM_ICMPSORTOPT		0x00000200
#define IM_NOKANJISORTOPT	0x00000400
#define IM_ALLREVSORTOPT	0x00000800
#define IM_GROUPREVSORTOPT	0x00001000
#define IM_SUBREVSORTOPT	0x00002000

#define WM_TASKBARNOTIFY		(WM_USER + 101)   //托盘通知消息
// #define WM_IM_INITICON			(WM_USER + 102)
// #define WM_RECVDLG_OPEN			(WM_USER + 110)
// #define WM_RECVDLG_EXIT			(WM_USER + 111)
// #define WM_RECVDLG_FILEBUTTON	(WM_USER + 112)
// #define WM_SENDDLG_OPEN			(WM_USER + 121)
// #define WM_SENDDLG_CREATE		(WM_USER + 122)
// #define WM_SENDDLG_EXIT			(WM_USER + 123)
// #define WM_SENDDLG_HIDE			(WM_USER + 124)
// #define WM_SENDDLG_RESIZE		(WM_USER + 125)
#define WM_UDPEVENT				(WM_USER + 130)
#define WM_TCPEVENT				(WM_USER + 131)
// #define WM_REFRESH_HOST			(WM_USER + 140)
// #define WM_MSGDLG_EXIT			(WM_USER + 150)
// #define WM_DELMISCDLG			(WM_USER + 151)
// #define WM_HIDE_CHILDWIN		(WM_USER + 160)
// #define WM_EDIT_DBLCLK			(WM_USER + 170)
// #define WM_DELAYSETTEXT			(WM_USER + 180)
#define WM_SENDOBJEVENT			(WM_USER + 181)		//发送文件传输状态变更事件
#define WM_RECVOBJEVENT			(WM_USER + 182)		//发送文件传输状态变更事件

// CryptoAPI for VC4
#ifndef MS_DEF_PROV
typedef unsigned long HCRYPTPROV;
typedef unsigned long HCRYPTKEY;
typedef unsigned long HCRYPTHASH;
typedef unsigned int ALG_ID;
#define ALG_TYPE_RSA			(2 << 9)		//即0000010000000000	16位
#define ALG_TYPE_BLOCK			(3 << 9)
#define ALG_CLASS_DATA_ENCRYPT	(3 << 13)
#define ALG_CLASS_HASH			(4 << 13)
#define ALG_CLASS_KEY_EXCHANGE	(5 << 13)		//即1110000000000000	16位
#define ALG_SID_RSA_ANY			0
#define ALG_TYPE_ANY			0
#define ALG_SID_RC2				2
#define ALG_SID_MD5				3
#define CALG_RSA_KEYX			(ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_RSA|ALG_SID_RSA_ANY) //1110010000000000
#define CALG_RC2				(ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_RC2)
#define CALG_MD5				(ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD5)
#define CRYPT_EXPORTABLE		0x00000001
#define PROV_RSA_FULL			1
#define MS_DEF_PROV				"Microsoft Base Cryptographic Provider v1.0"
#define MS_ENHANCED_PROV		"Microsoft Enhanced Cryptographic Provider v1.0"
#define CUR_BLOB_VERSION		0x02
#define SIMPLEBLOB				0x1
#define PUBLICKEYBLOB			0x6
#define PRIVATEKEYBLOB          0x7
#define CRYPT_NEWKEYSET			0x00000008
#define CRYPT_DELETEKEYSET      0x00000010
#define CRYPT_MACHINE_KEYSET    0x00000020
#define AT_KEYEXCHANGE			1
#define AT_SIGNATURE			2
#define KP_EFFECTIVE_KEYLEN		19	// for CryptSetKeyParam

typedef struct _CRYPTOAPI_BLOB {
	DWORD   cbData;
	BYTE    *pbData;
} DATA_BLOB, *PDATA_BLOB;
#endif //MS_DEF_PROV

#define DEFAULTGROUPNAMEEX_STR	_T("我的好友")
#define HEADGRAPHDIR_STR		_T("HeadGraph")
#define DEFAULTSAVEDIR_STR		_T("RecvDir")

#define APPNAME_STR				_T("FxIM - 局域网通讯")
#define MSGCAPTION_STR			_T("FxIM")
#define MSGPATHTOOLONG_STR		_T("路径太长")
#define MSGNOTSAFEPATH_STR		_T("路径不对")
#define MSGCREATEFAIL_STR		_T("创建文件失败")
#define MSGOVERWRITE_STR		_T("文件（夹）已存在\n是否覆盖?")
#define MSGWRITEFAIL_STR		_T("文件写入失败")



#define CRYPTPROTECT_UI_FORBIDDEN		0x1
#define CRYPTPROTECT_LOCAL_MACHINE		0x4
#define CRYPTPROTECT_CRED_SYNC			0x8
#define CRYPTPROTECT_AUDIT				0x10
#define CRYPTPROTECT_VERIFY_PROTECTION	0x40

#define SKEY_HEADER_SIZE	12

#define PRIV_BLOB_USER			0x0001
#define PRIV_BLOB_DPAPI			0x0002
#define PRIV_BLOB_RAW			0x0003
#define PRIV_SEED_HEADER		"FxIM:"
#define PRIV_SEED_HEADER_LEN	6
#define PRIV_SEED_LEN			(PRIV_SEED_HEADER_LEN + (128/8))	// 128bit seed



// General define
#define MAX_MACLEN      16							// e.g. 00-50-56-C0-00-08
#define MAX_SOCKBUF		65536
#define MAX_UDPBUF		16384
#define MAX_CRYPTLEN	((MAX_UDPBUF - MAX_BUF) / 2)
#define MAX_BUF			1024
#define MAX_NAMEBUF		50
#define MAX_LANGBUF		10
#define MAX_LISTBUF		(MAX_NAMEBUF * 3 + 50)
#define MAX_ANSLIST		100

#define IM_DEFAULT_VIEWMAX		8388608				// (8 * 1024 * 1024) 8MB
#define IM_DEFAULT_TRANSMAX		65536				// (64 * 1024)	64KB
#define IM_DEFAULT_TCPBUFMAX	262144				// (256 * 1024)	256KB

#define FT_STRICTDATE	0x00000001
#define RS_REALTIME		0x00000001

#define	FACE_CTRL_SEL		WM_USER + 1

#define IPMSG_AVATAR	_T("misc\\ipmsg.png")
#define DEFAULT_AVATAR	_T("misc\\default_avatar.png")

struct TListObj {
	TListObj *prior, *next;
};

class TList {
protected:
	TListObj	top;

public:
	TList(void);
	virtual void		AddObj(TListObj *obj);
	static  void		DelObj(TListObj *obj);
	virtual TListObj	*TopObj(void);
	virtual TListObj	*NextObj(TListObj *obj);
};

//////////////////////////////////////////////////////////////////////////
//struct PubKey RSA公钥
//////////////////////////////////////////////////////////////////////////
struct PubKey 
{
	BYTE	*key;
	int		keyLen;
	int		e;      //exponent(指数，幂)
	int		capa;

	PubKey(void);
	~PubKey(void);
	const BYTE *Key(void) { return key; }
	PubKey	&operator = (PubKey &pubKey);
	void	ReSet(void);
	void	Set(BYTE *key, int keyLen, int e, int capa);
	void	SetByBlob(BYTE *blob, int _capa);	
	int		KeyBlobLen(void);
	BOOL	KeyBlob(BYTE *blob, int maxLen, int *len);	
};

//////////////////////////////////////////////////////////////////////////
//struct AddrObj
//////////////////////////////////////////////////////////////////////////
struct AddrObj : public TListObj
{
	ULONG addr;
	int	  portNo;

	AddrObj();
	AddrObj(ULONG addr, int portNo);
};

//////////////////////////////////////////////////////////////////////////
//struct HostSub 主机部分信息
//////////////////////////////////////////////////////////////////////////
struct HostSub
{
	char	userName[MAX_NAMEBUF];
	char	hostName[MAX_NAMEBUF];	
	ULONG	addr;
	USHORT	portNo;

	BOOL	IsSameHostSub(HostSub *hostSub);
	BOOL	operator == (HostSub &hostSub);
	HostSub	&operator = (HostSub &hostSub);
};

//////////////////////////////////////////////////////////////////////////
//struct HostSet 主机设置信息
//////////////////////////////////////////////////////////////////////////
#define HOSTSET_ADDNEW			0x0001		//在存入XML文件时新添加一项
#define HOSTSET_NICKNAMEEX		0x0002
#define HOSTSET_GROUPNAMEEX		0x0004

struct HostSet
{
	char	hostName[MAX_NAMEBUF];			//前两项确定一台主机
	ULONG	addr;

	char	nickNameEx[MAX_NAMEBUF];		//备注名
	char	groupNameEx[MAX_NAMEBUF];		//备注组名	

	UINT	status;							//当写入时，要写入哪些数据，每次重复所有都写入的方法不可取
	time_t	lastLoginTime;					//最后登录时间，超过一定时间此用户如再无登录，删除该配置，避免无用数据继续存储在XML中增加程序负担

	HostSet();
	~HostSet();
};

//////////////////////////////////////////////////////////////////////////
//struct Host 主机信息
//////////////////////////////////////////////////////////////////////////
struct Host
{
	HostSub hostSub;
	HostSet *hostSet;					//主机设置信息，新增
	char	nickName[MAX_NAMEBUF];      //昵称
	char	groupName[MAX_NAMEBUF];     //工作组名
	ULONG	hostStatus;
	time_t	updateTime;
	int		priority;			        //优先权，这项取消掉，本程序中不考虑这些
	int		refCnt;						//re file cnt 文件发送主机重复使用次数
	PubKey	pubKey; 
	int		cryptSpec;
	char	avatar[MAX_PATH];			//头像文件名，新增
	char	mac[MAX_MACLEN];			//mac地址
	UINT	tag;						// 新增，用于保存用户列表结点指点
	HICON	hIconAvatar;				// 头像

	Host(void);
	~Host(void);
	void	Init(HostSub *hostSub, ULONG command, time_t update_time, char *nickName, char *groupName);
	int		RefCnt(int cnt = 0) { return refCnt += cnt; }
	char*	NickName(void) {return  *nickName ? nickName : hostSub.userName;}
	char*	NickNameEx() {return (hostSet && *hostSet->nickNameEx != 0) ? hostSet->nickNameEx : NickName();}
};

//////////////////////////////////////////////////////////////////////////
//class CHostMng
//////////////////////////////////////////////////////////////////////////
class THosts {
public:
	enum Kind { NAME, ADDR, NAME_ADDR, MAX_ARRAY };
	BOOL enable[MAX_ARRAY];

protected:
	int		hostCnt;
	Host	**array[MAX_ARRAY];
	Host	*Search(Kind kind, HostSub *hostSub, int *insertIndex=NULL);
	int		Cmp(HostSub *hostSub1, HostSub *hostSub2, Kind kind);

public:
	THosts(void);
	~THosts();

	void	Enable(Kind kind, BOOL _enable) { enable[kind] = _enable; }
	BOOL	AddHost(Host *host);
	BOOL	DelHost(Host *host);
	int		HostCnt(void) { return hostCnt; }
	Host	*GetHost(int index, Kind kind=NAME) { return array[kind][index]; }
	Host	*GetHostByName(HostSub *hostSub) { return enable[NAME] ? Search(NAME, hostSub) : NULL; }
	Host	*GetHostByAddr(HostSub *hostSub) { return enable[ADDR] ? Search(ADDR, hostSub) : NULL; }
	Host	*GetHostByNameAddr(HostSub *hostSub) { return enable[NAME_ADDR] ? Search(NAME_ADDR, hostSub) : NULL; }
	int		PriorityHostCnt(int priority, int range=1);
};

//////////////////////////////////////////////////////////////////////////
//struct SendEntry
//////////////////////////////////////////////////////////////////////////
enum SendStatus { ST_GETCRYPT=0, ST_MAKECRYPTMSG, ST_MAKEMSG, ST_SENDMSG, ST_DONE };
struct SendEntry
{
	Host		*host;
	SendStatus	status;
	ULONG		command;
	int			msgLen;
	char		msg[MAX_UDPBUF];
};


//////////////////////////////////////////////////////////////////////////
//struct MsgBuf  消息
//////////////////////////////////////////////////////////////////////////
struct MsgBuf
{ 
	HostSub	hostSub;
	int		version;
	USHORT	portNo;
	ULONG	packetNo;
	ULONG	command;
	int		exOffset;			// expand message offset in msgBuf
	time_t	recvTime;			// msg recv time [11/28/2014 ybt]
	int		dummy;
	char	msgBuf[MAX_UDPBUF];

	MsgBuf(void);
	void	Init(MsgBuf *org);	
};

//////////////////////////////////////////////////////////////////////////
//struct RecvBuf   接收信息
//////////////////////////////////////////////////////////////////////////
struct RecvBuf
{
	sockaddr_in	addr;
	int			addrSize;
	int			size;
	char		msgBuf[MAX_UDPBUF];
};

//////////////////////////////////////////////////////////////////////////
//struct ConnectInfo  连接信息
//////////////////////////////////////////////////////////////////////////
struct ConnectInfo : public TListObj {
	SOCKET	sd;
	ULONG	addr;
	USHORT	port;
	BOOL	server;
	BOOL	complete;
	DWORD	startTick;
	DWORD	lastTick;
};

//////////////////////////////////////////////////////////////////////////
//struct FileInfo
//////////////////////////////////////////////////////////////////////////
struct FileInfo
{
	int			id;			
	char		*fname;
	char		*fname_ext;	//for recv dir
	UINT		attr;
	UINT		exattr;		// 标识文件类型(头像、聊天图片等) [12/15/2014 ybt]
	_int64		size;		//int 为32位数据，而_int64可表示大于4GB的文件
	time_t		mtime;
	time_t		atime;		//
	time_t		crtime;		//创建时间
	BOOL		isSelected;	//

	FileInfo(int _id=0) { id=_id; fname_ext=fname=NULL; size=0; mtime=0; }
	FileInfo(const FileInfo& org) { fname=NULL; *this = org; }
	~FileInfo() { if (fname) free(fname); }

	const char *Fname() { return fname ? fname : fname_ext; }
	void SetFname(const char *_fname) { if (fname) free(fname); fname = (char *)_strdup(_fname); }
	FileInfo& operator =(const FileInfo& org) { id=org.id; SetFname(org.fname); attr=org.attr; exattr=org.exattr; size=org.size; mtime=org.mtime; atime=org.atime; crtime=org.crtime; isSelected=org.isSelected; return *this; }
};


//////////////////////////////////////////////////////////////////////////
//struct ShareInfo
//////////////////////////////////////////////////////////////////////////
struct ShareInfo : public TListObj
{
	ULONG		packetNo;		
	Host		**host;			//支持向多个主机发送，虽然本程序目前是一对一，还是保留吧
	int			hostCnt;		// 
	char		*transStat;		//
	FileInfo	**fileInfo;		//文件列表（多个文件/文件夹发送）
	int			fileCnt;
	FILETIME	attachTime;

	ShareInfo(ULONG packetNo = 0);
	void LinkList(ShareInfo *top);	
};


//////////////////////////////////////////////////////////////////////////
//struct AcceptFileInfo
//////////////////////////////////////////////////////////////////////////
struct AcceptFileInfo 
{
	FileInfo	*fileInfo;
	Host		*host;
	_int64		offset;
	ULONG		packetNo;
	ULONG		command;
	FILETIME	attachTime;
};

struct ShareCntInfo {
	int		hostCnt;
	int		fileCnt;
	int		dirCnt;
	int		transferCnt;
	int		doneCnt;
	int		packetCnt;
	_int64	totalSize;
};


struct UrlObj : public TListObj {
	char	protocol[MAX_NAMEBUF];
	char	program[MAX_PATH];
};

ULONG ResolveAddr(const char *_host);

class TBroadcastObj : public TListObj {
	char	*host;
	ULONG	addr;
public:
	TBroadcastObj(const char *_host=NULL, ULONG _addr=0) { host=_strdup(_host); addr=_addr; }
	~TBroadcastObj() { if (host) free(host); };
	const char *Host() { return	host; }
	ULONG	Addr(BOOL realTime=FALSE) { return realTime ? (addr = ResolveAddr(host)) : addr; }
};

class TBroadcastList : public TList {
public:
	TBroadcastList() {}
	~TBroadcastList() { Reset(); }
	void Reset(void);

	void SetHostRaw(const char *host, ULONG addr=0)
	{ TBroadcastObj *obj = new TBroadcastObj(host, addr); AddObj(obj); }

	TBroadcastObj *Top() { return (TBroadcastObj *)TopObj(); }
	TBroadcastObj *Next(TBroadcastObj *obj) { return (TBroadcastObj *)NextObj(obj); }
};

struct CharFormatLite
{
	CharFormatLite(void) 
	{
		nSize = 9;
		bBold = FALSE;
		bItalic = FALSE;
		bUnderLine = FALSE;
		crText = RGB(0,0,0);
		_tcsncpy_s(szFaceName, _T("宋体"), LF_FACESIZE);
	}
	int			nSize;						// 字体大小
	bool		bBold;						// 是否加粗
	bool		bItalic;					// 是否倾斜
	bool		bUnderLine;					// 是否带下划线
	COLORREF	crText;						// 字体颜色
	TCHAR		szFaceName[LF_FACESIZE];	// 字体名称
};


enum FileStatus { 
	FS_DIRFILESTART, 
	FS_OPENINFO, 
	FS_FIRSTINFO, 
	FS_NEXTINFO, 
	FS_MAKEINFO, 
	FS_TRANSINFO, 
	FS_TRANSFILE, 
	FS_ENDFILE, 
	FS_COMPLETE,	
	FS_ERROR,			// 传输错误
	FS_ALREADYEXISTS,	// 已经存在
	FS_CANCEL			// 手动中断传输 [12/17/2014 ybt]
};

struct SendFileObj : public TListObj 
{
	ConnectInfo	*conInfo;
	FileInfo	*fileInfo;
	FILETIME	attachTime;

	UINT		command;
	BOOL		isDir;
	_int64		fileSize;
	_int64		offset;
	_int64		totalTrans;			//add
	int			totalFiles;			//add 已传输的文件数
	char		*mapAddr;
	HANDLE		hFile;
	HANDLE		hMap;
	HANDLE		hThread;

	Host		*host;
	int			packetNo;
	FileStatus	status;

	int			dirCnt;
	HANDLE		*hDir;				// FindFirst handle
	char		path[MAX_PATH];

	int			headerLen;
	int			headerOffset;
	char		header[MAX_BUF];	// for dirinfo
	UINT_PTR	tag;				// 用户定义，一般指向CSendFileElmentUI指针
	HWND		hWndTcp;			// 传输窗口，一般是主窗口
	HWND		hWndTrans;			// 消息通知窗口，普通文件(夹)的通知窗口是聊天窗口。头像文件等是主窗口

	WIN32_FIND_DATA	fdata;	
};


struct RecvFileObj : public TListObj {
	ConnectInfo	*conInfo;
	FileInfo	*fileInfo;			// 和SendFileObj不同，它重新new了一个空间
	

	BOOL		isDir;
	FileInfo	curFileInfo;

	_int64		offset;
	_int64		woffset;
	char		*recvBuf;
	HANDLE		hFile;
	HANDLE		hThread;

	int			infoLen;
	int			dirCnt;

	_int64		totalTrans;
	DWORD		startTick;
	DWORD		lastTick;
	int			totalFiles;
	FileStatus	status;
	char		saveDir[MAX_PATH];
	char		path[MAX_PATH];
	char		info[MAX_BUF];	// for dirinfo buffer

	int			packetNo;			// 
//	int			index;				// fileInfo在发送方ShareInfo中的索引值
//	ShareInfo*	shareInfo;
	UINT_PTR	tag;				// 用户定义，一般指向CRecvFileElmentUI指针
	HWND		hWndTcp;			// 传输窗口，一般是主窗口
	HWND		hWndTrans;			// 消息通知窗口，普通文件(夹)的通知窗口是聊天窗口。头像文件等是主窗口
};


#define IsNewShell()	(LOBYTE(LOWORD(GetVersion())) >= 4)
#define IsWin95()		(LOBYTE(LOWORD(GetVersion())) >= 4 && GetVersion() >= 0x80000000)
#define IsWinNT()		(LOBYTE(LOWORD(GetVersion())) >= 4 && GetVersion() < 0x80000000)
#define IsWin2K()		(LOBYTE(LOWORD(GetVersion())) >= 5 && GetVersion() < 0x80000000)
#define IsWinXP()		(LOBYTE(LOWORD(GetVersion())) >= 6 && GetVersion() < 0x80000000)
#define IsWin31()		(LOBYTE(LOWORD(GetVersion())) == 3 && HIBYTE(LOWORD(GetVersion())) < 20)
#define IsNT350()		(LOBYTE(LOWORD(GetVersion())) == 3 && HIBYTE(LOWORD(GetVersion())) == 50)


//////////////////////////////////////////////////////////////////////////
//全局函数
//////////////////////////////////////////////////////////////////////////
void ShowErrorInfo(DWORD errorCode, LPCTSTR exMsg = NULL); 
void MakeHash(const BYTE *data, int len, char *hashStr);
void bin2hexstr(const BYTE *bin, int len, char *buf);
void bin2hexstr_bigendian(const BYTE *bin, int len, char *buf);
BOOL hexstr2bin(const char *buf, BYTE *bindata, int maxlen, int *len);
BOOL hexstr2bin_bigendian(const char *buf, BYTE *bindata, int maxlen, int *len);
char *separate_token(char *buf, char separetor, char **handle=NULL);

_int64 hex2ll(char *src);

void ForcePathToFname(char *dest, const char *src);
BOOL PathToFname(char *dest, const char *src);
BOOL PathToDir(char *dest, const char *src);
int	 MakePath(char *dest, const char *dir, const char *file);
BOOL IsSafePath(const char *fullPath, const char *fname);
_int64 GetFolderSize(const char *fname);
BOOL GetFileInfomation(const char *path, WIN32_FIND_DATA *fdata);
int	 MakeSizeString(LPTSTR dest, _int64 size);
int	 MakeTransRateStr(char *dest, DWORD ticks, _int64 cur_size, _int64 total_size);
void MakeDirTransRateStr(char *buf, DWORD ticks, _int64 cur_size, int files);

#define UNIXTIME_BASE	((_int64)0x019db1ded53e8000)
inline time_t FileTime2UnixTime(FILETIME *ft) {
	return	(time_t)((*(_int64 *)ft - UNIXTIME_BASE) / 10000000);
}
inline void UnixTime2FileTime(time_t ut, FILETIME *ft) {
	*(_int64 *)ft = (_int64)ut * 10000000 + UNIXTIME_BASE;
}


#endif // _BASE_DEF_H_