
#include "StdAfx.h"
#include "Utils/Config.h"
#include "Utils/Base64.h"


//////////////////////////////////////////////////////////////////////////
//struct Config
//////////////////////////////////////////////////////////////////////////
#define CFGFILE_STR					_T("FxIM.xml")
#define ROOT_STR					_T("FxIM")

//CONFIG_PERSONAL
#define PERSONAL_STR				_T("Personal")
#define AVATAR_STR					_T("Avatar")
#define NICKNAME_STR				_T("NickName")
#define GROUPNAME_STR				_T("GroupName")

//CONFIG_GENERAL
#define GENERAL_STR					_T("General")
#define USERLISTSTYLE_STR			_T("UserListStyle")
#define USERITEMTEXTSTYLE_STR		_T("UserItemTextStyle")
#define USERITEMATTRIBUTESTYLE_STR	_T("UserItemAttributeStyle")
#define RECVMSGNOTAUTOOPEN_STR		_T("RecvMsgNotAutoOpen")
#define RECVFILEAUTOOPEN_STR		_T("RecvFileAutoOpen")
#define RECVMSGSOUND_STR			_T("RecvMsgSound")
#define RECVMSGSOUNDFILE_STR		_T("RecvMsgSoundFile")
#define STARTSHOWHIDE_STR			_T("StartShowHide")			//启动后最小化
#define MAINWNDALWAYSTOP_STR		_T("MainWndAlwaysTop")

//CONFIG_CRYPT
#define CRYPT_STR					_T("Crypt")
#define PRIVBLOB_STR				_T("PrivBlob")
#define PRIVTYPE_STR				_T("PrivType")
#define PRIVSEED_STR				_T("PrivEncryptSeed")
#define PRIVSEEDLEN_STR				_T("PrivEncryptSeedLen")


//CONFIG_WINSIZE
#define WINSIZE_STR					_T("WinSize")
#define SENDMSGHEIGHT_STR			_T("SendMsgHeight")
#define CHATSIZECX_STR				_T("ChatSizeCx")
#define CHATSIZECY_STR				_T("ChatSizeCy")
#define MAINRECTLEFT_STR			_T("MainRectLeft")
#define MAINRECTTOP_STR				_T("MainRectTop")
#define MAINRECTRIGHT_STR			_T("MainRectRight")
#define MAINRECTBOTTOM_STR			_T("MainRectBottom")

//CONFIG_HOSTSUB
#define HOSTSUBLIST_STR				_T("HostSubList")
#define HOSTSUB_STR					_T("HostSub")
#define HOSTNAME_STR				_T("HostName")			//主机名 (只存储主机名的IP地址，因为主机名和IP地址足以判断一台主机信息，本人也想另外再声明一个结构体，只存储这五个数据，但无奈有点麻烦，先暂时用HostSub这个结构体吧)
#define HOSTADDR_STR				_T("HostAddr")			//IP地址
#define NICKNAMEEX_STR				_T("NickNameEx")		//备注昵称（例：用户A将B主机的备注名改为王五，当A再次上线时，程序从存储的XML的文件中读取到B的备注名为王五，而不需要再次设置B的备注名）
#define GROUPNAMEEX_STR				_T("GroupNameEx")		//同上
#define HEADGRAPHNAME_STR			_T("HeadGraph")			//用户头像文件名
#define ABSENCENUM_STR				_T("AbsenceNum")		//连续不上线的次数
#define MAX_ABSENCENUM				30						//连续不上线的次数最大值

//CONFIG_NETWORK
#define NETWORK_STR					_T("Network")
#define ADAPTER_STR					_T("Adapter")			//指定网卡
#define DAILUP_STR					_T("DailupCheck")		//是否是拨号网络

//CONFIG_SKIN
#define SKIN_STR					_T("Skin")
#define BKIMAGE_STR					_T("BkImage")
#define BKCOLOR_STR					_T("BkColor")



#define IM_DEFAULT_RETRYMSEC		1500
#define IM_DEFAULT_DELAY			500

Config::Config()
{
	::GetModuleFileName(NULL, szAppDir, MAX_PATH);
	::PathRemoveFileSpec(szAppDir);

	privBlob = NULL;
	privEncryptSeed = NULL;	

	// CryptProtectData is available only Win2K/XP
	privEncryptType = IsWin2K() ? PRIV_BLOB_DPAPI : PRIV_BLOB_RAW;
	hPrivKey = 0;
	hSmallPrivKey = 0;

	hosts.Enable(THosts::NAME_ADDR, TRUE);
	hosts.Enable(THosts::ADDR, TRUE);
	fileHosts.Enable(THosts::NAME_ADDR, TRUE);
	nResolveOpt = 0;
	nDelayTime = IM_DEFAULT_DELAY;
	bExtendEntry = TRUE;
	bDialUpCheck = FALSE;
	bAbsenceCheck = TRUE;

	TransMax = IM_DEFAULT_TRANSMAX;
	ViewMax = IM_DEFAULT_VIEWMAX;
	fileTransOpt = 0;

	*szLastOpenDir = 0;

	SHGetSpecialFolderPath(NULL, szPersonalDir, CSIDL_PERSONAL, FALSE);
	_tcscat(szPersonalDir, _T("\\FxIM\\"));
	MakeSureDirectoryPathExists(szPersonalDir);

	wsprintf(szCfgFilePath, "%s%s", szPersonalDir, CFGFILE_STR);

	wsprintf(szDefaultFileSaveDir, _T("%sFileRecv\\"), szPersonalDir);
	MakeSureDirectoryPathExists(szDefaultFileSaveDir);

	wsprintf(szChatImageSaveDir, _T("%sChatImage\\"), szPersonalDir);
	MakeSureDirectoryPathExists(szChatImageSaveDir);

	wsprintf(szAvatarSaveDir, _T("%sAvatar\\"), szPersonalDir);
	MakeSureDirectoryPathExists(szAvatarSaveDir);

	*szNickName = 0;
	*szGroupName = 0;
	_tcsncpy(szAvatar, DEFAULT_AVATAR, MAX_PATH);
	szAbsenceHead = NULL;
	szAbsenceStr = NULL;

	*szBgImage = 0;
	dwBgColor = 0xFF76A8D0;				// Duilib 控件的背景色默认值为0x00000000

	bWantReturnSend = TRUE;
	bSendCheckOpt = TRUE;

	pIpAdapterInfo = NULL;
	pIpAdapterInfoUse = NULL;
	*szAdapterNameUse = 0;
//	memcpy(szAdapterNameUse, "{A1F9F8C2-A772-46D7-A366-7B30D8B9C8DD}", MAX_PATH);
//	memset(AddressUse, 0, MAX_ADAPTER_ADDRESS_LENGTH);
//	memcpy(AddressUse, "\x38\x59\xF9\xE0\xA3\xD0\x00\x00", MAX_ADAPTER_ADDRESS_LENGTH);
	nAddr = ADDR_ANY;
	nPort = IM_DEFAULT_PORT;

	nWriteFlag = 0;

	Init();
}

Config::~Config()
{
	// 删除网卡信息。
	if (pIpAdapterInfo != NULL)
	{
		 free(pIpAdapterInfo);
		pIpAdapterInfo = NULL;
	}
	pIpAdapterInfoUse = NULL;

	// 删除私钥
	if (privBlob != NULL) {
		delete [] privBlob;			//因为privBlob是内部变量，[]可以省略
	}
	if (privEncryptSeed != NULL) {
		delete [] privEncryptSeed;
	}
	if (hPrivKey) {
		CryptDestroyKey(hPrivKey);
	}
	if (hSmallPrivKey) {
		CryptDestroyKey(hSmallPrivKey);
	}
}

void Config::Init()
{
	// 读取配置
	ReadConfig();			

	// 匹配设置网卡的物理地址
	GetAdapters();
	MatchAdapter();
	if (pIpAdapterInfoUse != NULL)
	{
		nAddr = inet_addr(pIpAdapterInfoUse->IpAddressList.IpAddress.String);
	}
}

// http://www.cnblogs.com/L-hq815/archive/2012/08/04/2622829.html
BOOL Config::GetAdapters()
{
	ULONG	nRel;
	ULONG	nSize;

	nSize = sizeof(IP_ADAPTER_INFO);
	pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(nSize);;
	if (pIpAdapterInfo == NULL) {
		return FALSE;
	}

	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中nSize参数既是一个输入量也是一个输出量
	nRel = GetAdaptersInfo(pIpAdapterInfo, &nSize);

	// 如果函数返回的是ERROR_BUFFER_OVERFLOW,则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出nSize,表示需要的空间大小
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_INFO)realloc(pIpAdapterInfo, nSize);
		if (pIpAdapterInfo == NULL) {
			return FALSE;
		}
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		nRel = GetAdaptersInfo(pIpAdapterInfo, &nSize);
	}

	return nRel == ERROR_SUCCESS;
}

void Config::MatchAdapter()
{
	PIP_ADAPTER_INFO	pAdapter = pIpAdapterInfo;

	// 如果之前设置了指定网卡
	if (*szAdapterNameUse != 0)
	{
		while (pAdapter != NULL) 
		{
			if (_tcscmp(pAdapter->AdapterName, szAdapterNameUse) == 0) {
				pIpAdapterInfoUse = pAdapter;
				break;
			}
			pAdapter = pAdapter->Next;
		}		
	}
	// 如果到最后没有匹配到之前设置的网卡，则清空配置中的网卡信息
	if (pIpAdapterInfoUse == NULL) {
		//memset(AddressUse, 0, MAX_ADAPTER_ADDRESS_LENGTH);
		*szAdapterNameUse = 0;
		nWriteFlag = CONFIG_NETWORK;
	}
}

BOOL Config::IsByteNull(BYTE* b, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (b[i] != 0) {
			return FALSE;
		}
	}
	return TRUE;
}


BOOL Config::ReadConfig()
{
	TiXmlDocument xmlDoc;

	if (!PathFileExists(szCfgFilePath) || !xmlDoc.LoadFile(szCfgFilePath)) {
		return TRUE;
	}

	TiXmlElement* rootElement = xmlDoc.RootElement();
	if (rootElement == NULL) {
		return FALSE;
	}

	ReadCrypt(rootElement);
	ReadPersonal(rootElement);
	ReadNetwork(rootElement);
	ReadSkin(rootElement);

	return TRUE;
}


BOOL Config::WriteConfig(int nFlag /* = CONFIG_ALL */)
{
	TiXmlDocument	xmlDoc;
	TiXmlElement*	rootElement;

	if (nFlag == 0) { return TRUE; }
	if (!PathFileExists(szCfgFilePath) || !xmlDoc.LoadFile(szCfgFilePath)) {
		rootElement = new TiXmlElement(ROOT_STR);
		xmlDoc.LinkEndChild(rootElement);
		nFlag = CONFIG_ALL;
	}
	else {	
		rootElement = xmlDoc.RootElement();
	}
	
	if (nFlag & CONFIG_CRYPT) {	WriteCrypt(rootElement); }
	if (nFlag & CONFIG_PERSONAL) { WritePersonal(rootElement); }
	if (nFlag & CONFIG_NETWORK) { WriteNetwork(rootElement); }
	if (nFlag * CONFIG_SKIN) { WriteSkin(rootElement); }
	
	//保存XML文件
	if (!xmlDoc.SaveFile(szCfgFilePath)) {
		::MessageBox(NULL, _T("Save config failed"), APPNAME_STR, MB_OK| MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}


void Config::SetElementValue(const char* elementName, const char* szValue, 
							 TiXmlElement* parentElement)
{
	TiXmlElement	*tixmlElement;
	TiXmlNode		*tixmlText;

	if((tixmlElement = parentElement->FirstChildElement(elementName)) == NULL)
	{
		// 节点不存在，创建之
		tixmlElement = new TiXmlElement(elementName);
		parentElement->LinkEndChild(tixmlElement);	
	}
	if ((tixmlText = tixmlElement->FirstChild()) == NULL) 
	{
		tixmlText = new TiXmlText(szValue);
		tixmlElement->LinkEndChild(tixmlText);
	}
	else {
		tixmlText->SetValue(szValue);
	}	
}

void Config::SetElementValue(const char* elementName, unsigned long nValue, 
							 TiXmlElement* parentElement)
{
	char	szDigital[12] = {0};

	ltoa(nValue, szDigital, 16);				// 有符号整数
	SetElementValue(elementName, szDigital, parentElement);
}

BOOL Config::GetElementValue(const char* elementName, char* szValue, int nMaxCount, 
							const TiXmlElement* parentElement)
{
	const TiXmlElement	*tixmlElement;
	const TiXmlNode		*tixmlText;

	if((tixmlElement = parentElement->FirstChildElement(elementName)) == NULL) {
		return FALSE;
	}
	if ((tixmlText = tixmlElement->FirstChild()) == NULL) {
		return FALSE;
	}
	memset(szValue, 0, nMaxCount);
	strncpy(szValue, tixmlText->Value(), nMaxCount - 1);

	return TRUE;
}


BOOL Config::GetElementValue(const char* elementName, unsigned long* pnValue, 
							 const TiXmlElement* parentElement)
{
	char	szDigital[12] = {0};
	long	nSize = 12;

	if (GetElementValue(elementName, szDigital, nSize, parentElement))
	{
		try {
			//_ttol("ff76a8d0") = 0
			//_ttol("0xff76a8d0") = 0
			//*pnValue = _ttol(szDigital);
			*pnValue = strtoul(szDigital, 0, 16);
			return TRUE;
		}
		catch (...) {
			return FALSE;
		}
	}

	return FALSE;
}


void Config::ReadCrypt(TiXmlElement* rootElement)
{
	TiXmlElement*	cryptElement;
	long			nSize;
	char			szBuf[MAX_BUF] = {0};

	if ((cryptElement = rootElement->FirstChildElement(CRYPT_STR)) == NULL) {
		return ;
	}

	if (GetElementValue(PRIVBLOB_STR, szBuf, MAX_BUF, cryptElement))
	{
		nSize = strlen(szBuf);
		privBlobLen = (nSize + 3) / 4 * 3;
		privBlob = new BYTE[privBlobLen];
		if (privBlob != NULL) {
			Base64Decode(privBlob, szBuf, nSize);
		}
	}

	if (GetElementValue(PRIVSEED_STR, szBuf, MAX_BUF, cryptElement))
	{
		nSize = strlen(szBuf);
		privEncryptSeedLen = (nSize + 3) / 4 * 3;
		privEncryptSeed = new BYTE[privEncryptSeedLen];
		if (privEncryptSeed != NULL) {
			Base64Decode(privEncryptSeed, szBuf, nSize);
		}
	}
	GetElementValue(PRIVTYPE_STR, (unsigned long*)&privEncryptType, cryptElement);
}


void Config::WriteCrypt(TiXmlElement* rootElement)
{
	TiXmlElement*	cryptElement;

	if ((cryptElement = rootElement->FirstChildElement(CRYPT_STR)) == NULL) {
		cryptElement = new TiXmlElement(CRYPT_STR);
		rootElement->LinkEndChild(cryptElement);
	}

	if (privBlob != NULL)
	{
		char* szPrivBlob = (char*)_alloca((privBlobLen + 2) / 3 * 4 + 1);
		if (szPrivBlob != NULL)
		{
			Base64Encode(szPrivBlob, privBlob, privBlobLen);
			SetElementValue(PRIVBLOB_STR, szPrivBlob, cryptElement);
		}
	}	

	if (privEncryptSeed != NULL)
	{
		char *szPrivEncryptSeed = (char*)_alloca((privEncryptSeedLen + 2) / 3 * 4 + 1);
		if (szPrivEncryptSeed != NULL)
		{
			Base64Encode(szPrivEncryptSeed, privEncryptSeed, privEncryptSeedLen);
			SetElementValue(PRIVSEED_STR, szPrivEncryptSeed, cryptElement);
		}
	}
	SetElementValue(PRIVTYPE_STR, privEncryptType, cryptElement);

}


void Config::ReadPersonal(TiXmlElement* rootElement)
{
	TiXmlElement*	personalElement;
	char			szBuf[MAX_BUF] = {0};

	if ((personalElement = rootElement->FirstChildElement(PERSONAL_STR)) == NULL) {
		return;
	}
	GetElementValue(AVATAR_STR, szAvatar, MAX_PATH, personalElement);
	GetElementValue(NICKNAME_STR, szNickName, MAX_NAMEBUF, personalElement);
	GetElementValue(GROUPNAME_STR, szGroupName, MAX_NAMEBUF, personalElement);

}

void Config::WritePersonal(TiXmlElement* rootElement)
{
	TiXmlElement*	personalElement;

	if ((personalElement = rootElement->FirstChildElement(PERSONAL_STR)) == NULL) {
		personalElement = new TiXmlElement(PERSONAL_STR);
		rootElement->LinkEndChild(personalElement);
	}

	SetElementValue(AVATAR_STR, szAvatar, personalElement);
	SetElementValue(NICKNAME_STR, szNickName, personalElement);
	SetElementValue(GROUPNAME_STR, szGroupName, personalElement);
}

void Config::ReadNetwork(TiXmlElement* rootElement)
{
	TiXmlElement*	networkElement;
	char			szBuf[MAX_BUF] = {0};

	if ((networkElement = rootElement->FirstChildElement(NETWORK_STR)) == NULL) {
		return;
	}
	GetElementValue(ADAPTER_STR, szAdapterNameUse, MAX_PATH, networkElement);
}

void Config::WriteNetwork(TiXmlElement* rootElement)
{
	TiXmlElement*	networkElement;

	if ((networkElement = rootElement->FirstChildElement(NETWORK_STR)) == NULL) {
		networkElement = new TiXmlElement(NETWORK_STR);
		rootElement->LinkEndChild(networkElement);
	}

	SetElementValue(ADAPTER_STR, szAdapterNameUse, networkElement);
}

void Config::ReadSkin(TiXmlElement* rootElement)
{
	TiXmlElement*	skinElement;
	char			szBuf[MAX_BUF] = {0};

	if ((skinElement = rootElement->FirstChildElement(SKIN_STR)) == NULL) {
		return;
	}
	GetElementValue(BKIMAGE_STR, szBgImage, MAX_PATH, skinElement);
	GetElementValue(BKCOLOR_STR, &dwBgColor, skinElement);
}

void Config::WriteSkin(TiXmlElement* rootElement)
{
	TiXmlElement*	skinElement;

	if ((skinElement = rootElement->FirstChildElement(SKIN_STR)) == NULL) {
		skinElement = new TiXmlElement(SKIN_STR);
		rootElement->LinkEndChild(skinElement);
	}

	SetElementValue(BKIMAGE_STR, szBgImage, skinElement);
	SetElementValue(BKCOLOR_STR, dwBgColor, skinElement);
}