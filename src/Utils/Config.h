#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "Utils/BaseDef.h"
#include "ThirdParty/tinyxml/tinyxml.h"

#define		KEY_REBUILD			0x0001
#define		KEY_DIAG			0x0002

// 配置选项
#define		CONFIG_ALL			0xFFFFFFFF
#define		CONFIG_CRYPT		0x00000001
#define		CONFIG_GENERAL		0x00000002
#define		CONFIG_NETWORK		0x00000004
#define		CONFIG_PERSONAL		0x00000008
#define		CONFIG_SKIN			0x00000010


#define		RS_REALTIME			0x00000001

struct Config
{
public:
	BOOL		ReadConfig();
	BOOL		WriteConfig(int nFlag = CONFIG_ALL);

public:
	//////////////////////加密与解密相关变量START/////////////////////////////
	HCRYPTPROV	hCsp;
	HCRYPTPROV	hSmallCsp;
	PubKey		pubKey;
	PubKey		smallPubKey;	
	HCRYPTKEY	hPrivKey;
	HCRYPTKEY	hSmallPrivKey;

	int			privBlobLen;
	int			privEncryptType;
	int			privEncryptSeedLen;
	BYTE		*privBlob;	
	BYTE		*privEncryptSeed;
	//////////////////////加密与解密相关信息END/////////////////////////////

	// 网络相关
	PIP_ADAPTER_INFO	pIpAdapterInfo;
	PIP_ADAPTER_INFO	pIpAdapterInfoUse;
	//BYTE				AddressUse[MAX_ADAPTER_ADDRESS_LENGTH];			// 网卡mac地址，即PIP_ADAPTER_INFO.Address，由于读写配置时需要转换，比较不便，并且有些网卡的地址可能都是00-00-00-00-00，不具备唯一性
	char				szAdapterNameUse[MAX_PATH];						// 网卡名字，即PIP_ADAPTER_INFO.AdapterName  MAX_ADAPTER_NAME_LENGTH + 4 = MAX_PATH
	ULONG				nAddr;											// 使用的IP地址
	USHORT				nPort;

	
	TList				dialUpList;
	TBroadcastList		broadcastList;
	THosts				hosts;
	THosts				fileHosts;
	int					nResolveOpt;
	int					nDelayTime;
	BOOL				bExtendEntry;
	BOOL				bDialUpCheck;
	BOOL				bAbsenceCheck;
	int					nAbsenceMax;
	int					nAbsenceChoice;
	int					TransMax;						//文件传输缓冲区大小
	int					ViewMax;						//文件传输时文件映射大小
	int					fileTransOpt;					//文件传输方式

	TCHAR				(*szAbsenceStr)[MAX_PATH];
	TCHAR				(*szAbsenceHead)[MAX_NAMEBUF];

	TCHAR				szLastOpenDir[MAX_PATH];
	TCHAR				szPersonalDir[MAX_PATH];		// 
	TCHAR				szDefaultFileSaveDir[MAX_PATH]; // 文件默认接收目录
	TCHAR				szChatImageSaveDir[MAX_PATH];	// 聊天图片目录
	TCHAR				szAvatarSaveDir[MAX_PATH];		// 头像目录

	TCHAR				szNickName[MAX_NAMEBUF];
	TCHAR				szGroupName[MAX_NAMEBUF];
	TCHAR				szAvatar[MAX_PATH];				// File path

	TCHAR				szBgImage[MAX_PATH];			// 背景图及背景色
	DWORD				dwBgColor;

	CharFormatLite		cfLite;							// 个人聊天字体设置
	BOOL				bWantReturnSend;				// 是否按Enter键发送消息
	BOOL				bSendCheckOpt;					// 发送消息给对方，是否需要对方发送回执。

	int					nWriteFlag;

	
private:
	TCHAR				szAppDir[MAX_PATH];
	TCHAR				szCfgFilePath[MAX_PATH];
private:
	void	SetElementValue(const char* elementName, const char* szValue,
				TiXmlElement* parentElement);
	void	SetElementValue(const char* elementName, unsigned long nValue, 
				TiXmlElement* parentElement);
	BOOL	GetElementValue(const char* elementName, char* szValue, int nMaxCount, 
				const TiXmlElement* parentElement);	
	BOOL	GetElementValue(const char* elementName, unsigned long* pnValue, 
				const TiXmlElement* parentElement);

	void	ReadCrypt(TiXmlElement* rootElement);
	void	WriteCrypt(TiXmlElement* rootElement);

	void	ReadPersonal(TiXmlElement* rootElement);
	void	WritePersonal(TiXmlElement* rootElement);

	void	ReadNetwork(TiXmlElement* rootElement);
	void	WriteNetwork(TiXmlElement* rootElement);

	void	ReadSkin(TiXmlElement* rootElement);
	void	WriteSkin(TiXmlElement* rootElement);
private:
	Config();
	~Config();

	void	Init();
	BOOL	GetAdapters();
	void	MatchAdapter();
	BOOL	IsByteNull(BYTE* b, int len);

	DECLARE_SINGLETON_CLASS(Config)
};

#endif //_CONFIG_H_