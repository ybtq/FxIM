#include "Utils/Base64.h"


void Base64Encode(char *dest, const unsigned char *src, int src_len)
{
	int i;
	int len;

	//编码表
	static const char EncodeTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	len = src_len - src_len % 3;

	for (i = 0; i < len; i += 3)
	{		
		*dest++ = EncodeTable[src[i] >> 2];
		*dest++ = EncodeTable[(src[i] << 4 | src[i +1] << 4) & 0x3F];
		*dest++ = EncodeTable[(src[i +1] << 2 | src[i +2] << 6) & 0x3F];
		*dest++ = EncodeTable[src[i +2] & 0x3F];
	}

	//对剩余数据(一个或两个BYTE)进行编码
	switch(src_len % 3) 
	{
	case 1:
		*dest++ = EncodeTable[(src[i] >> 2) & 0x3F];
		*dest++ = EncodeTable[(src[i] << 4) & 0x30];
		*dest++ = '=';
		*dest++ = '=';
		break;
	case 2:
		*dest++ = EncodeTable[(src[i] >> 2) & 0x3F];
		*dest++ = EncodeTable[((src[i] << 4) & 0x03) | (src[i + 1] >> 4) & 0xF0];
		*dest++ = EncodeTable[(src[i] << 2) & 0x3C];
		*dest++ = '=';
		break;
	}

	*dest = 0;
}

//BASE64码解密为BYTE
void Base64Decode(unsigned char *dest, const char *src, int src_len)
{
	int i;
	//解码表
	static const char DecodeTable[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		62, // '+'
		0, 0, 0,
		63, // '/'
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
		0, 0, 0, 0, 0, 0,
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
	};

	for (i = 0; i < src_len; i += 4)
	{
		*dest++ = DecodeTable[src[i]] << 2 | DecodeTable[src[i + 1]] >> 4;
		*dest++ = DecodeTable[src[i + 1]] << 4 | DecodeTable[src[i + 2]] >> 2;
		*dest++ = DecodeTable[src[i + 2]] << 6 | DecodeTable[src[i + 3]];
	}
	//*dest = 0;
}
