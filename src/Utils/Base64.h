#ifndef _BASE64_H_
#define _BASE64_H_

// 杨波涛  2011-01-12(Thu)
//感谢隗公万的文章《base64编码与解码》
//base64的编码原理就是将三个连续的8bit字符转换成四个6bit的字符.

// 编码，dest的空间大小为(src_len + 2) / 3 * 4 + 1。(+1是最后有一个结束符的空间)
void	Base64Encode(char *dest, const unsigned char *src, int src_len);

// 解码，dest的空间大小为src_len / 4 * 3。(src_len总是4的倍数)
void	Base64Decode(unsigned char *dest, const char *src, int src_len);

#endif //_BASE64_H_