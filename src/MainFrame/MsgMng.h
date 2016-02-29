// MsgMng.h: interface for the CMsgMng class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MSGMNG_H_
#define _MSGMNG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsgMng
{
private:
	CMsgMng();
	virtual ~CMsgMng();
	DECLARE_SINGLETON_CLASS(CMsgMng)
public:	
	BOOL AsyncSelect(HWND hWnd);
	BOOL WSockInit(void);
	void WSockTerm(void);      //terminate(ÖÕÖ¹)

	BOOL Accept(HWND hWnd, ConnectInfo *connectInfo);
	BOOL Connect(HWND hWnd, ConnectInfo *connectInfo);
	BOOL AsyncSelectConnect(HWND hWnd, ConnectInfo *connectInfo);
	void ConnectDone(HWND hWnd, ConnectInfo *connectInfo);

	int  MakeMsg(char *dest, int packetNo, ULONG command, const char *msg = NULL, const char *exMsg = NULL);
	int  MakeMsg(char *dest, ULONG command, const char *msg = NULL, const char *exMsg = NULL);
	BOOL UdpSend(ULONG addr, USHORT portNo, const char *udpMsg, int len);
	BOOL UdpSend(HostSub *hostSub, const char* udpMsg, int len);

	BOOL Send(ULONG addr, USHORT portNo, ULONG command, const char *msg = NULL, const char *exMsg = NULL);
	BOOL Send(HostSub *hostSub, ULONG command, const char *msg = NULL, const char *exMsg = NULL);
	BOOL Send(HostSub *hostSub, ULONG command, int iVal);

	BOOL Recv(MsgBuf *msg);
	BOOL UdpRecv(RecvBuf *recv);
	BOOL ResolveMsg(RecvBuf *buf, MsgBuf *msg);

	BOOL IsAvailableTcp(void) { return m_sockTcp != INVALID_SOCKET ? TRUE : FALSE; }
 
	HostSub *GetLocalHostSub(void) {return &m_hostSubLocal;}
	ULONG	MakePacketNo(void) {return m_packetNo++;}

	static int  LocalNewLineToUnix(char *dest, const char *src, int maxlen);
	static int  UnixNewLineToLocal(char *dest, const char *src, int maxlen);
protected:
	SOCKET          m_sockTcp;
	SOCKET          m_sockUdp;
	ULONG           m_packetNo;					//°ü±àºÅ
	HWND            m_hAsyncWnd;				//´ý×¢²áµÄ´°¿Ú
	HostSub			m_hostSubLocal;
};

#endif // _MSGMNG_H_
