#pragma once
#include<string>
#include<math.h>
#include<iostream>
#include<fstream>
#define CHUNK_BYTE 64
typedef unsigned int uint_32;

class MD5
{
public:
	MD5()
	{
		init();
	}

	//初始化：_k[64],和调用reset重置
	void init()
	{
		// 因为此处i从0开始，所以需要sin(i + 1)
		for (int i = 0; i < 64; ++i)
		{
			_k[i] = (uint_32)(abs(sin(i + 1.0))*pow(2.0, 32));
		}
		reset();
	}

	//重置:_a,_b,_c,_d
	void reset()
	{
		_a = 0x67452301;
		_b = 0xefcdab89;
		_c = 0x98badcfe;
		_d = 0x10325476;
		memset(_chunk, 0, CHUNK_BYTE);
		_lastByte = 0;
		_totalByte = 0;
	}

	//位运算函数 F G H I
	uint_32 F(uint_32 b, uint_32 c, uint_32 d)
	{
		return (b & c) | ((~b) & d);
	}
	uint_32 G(uint_32 b, uint_32 c, uint_32 d)
	{
		return (b & d) | (c & (~d));
	}
	uint_32 H(uint_32 b, uint_32 c, uint_32 d)
	{
		return b ^ c ^ d;
	}
	uint_32 I(uint_32 b, uint_32 c, uint_32 d)
	{
		return  c ^ (b | (~d));
	}

	//循环左移
	uint_32 left_shift(uint_32 number, int shiftNumber)
	{
		return ((number << shiftNumber) | (number >> (32 - shiftNumber)));
	}

	//计算一个数据块chunk的MD5值
	void cal_MD5(uint_32* chunk)
	{
		int a = _a;
		int b = _b;
		int c = _c;
		int d = _d;
		int tmp_4;
		int g;
		//将这个数据块一共处理4轮，每轮处理16次，每次处理四个字节
		for (int i = 0; i < 64; ++i)
		{
			if (i >= 0 && i <= 15)
			{
				g = i;
				tmp_4 = F(b, c, d);
			}
			else if (i >= 16 && i <= 31)
			{
				g = (5 * i + 1) % 16;
				tmp_4 = G(b, c, d);
			}
			else if (i >= 32 && i <= 47)
			{
				g = (3 * i + 5) % 16;
				tmp_4 = H(b, c, d);
			}
			else
			{
				g = (7 * i) % 16;
				tmp_4 = I(b, c, d);
			}

			int tmp_d = d;
			d = c;
			c = b;
			b = b + left_shift((a + tmp_4 + _k[i] + chunk[g]), _leftShift[i]);
			a = tmp_d;
		}
		//更新四个值
		_a += a;
		_b += b;
		_c += c;
		_d += d;
	}

	//给最后一个块数据添加填充位和比特位长度
	void cal_finalMD5()
	{
		char* p = _chunk + _lastByte;
		//填充冗余信息（最少填充一个字节，因为文件里最小单位是一个字节）
		*p++ = (char)0x80;
		int remain_byte = CHUNK_BYTE - _lastByte - 1;
		if (remain_byte < 8)
		{//如果后面不够64比特位（8字节），就把剩余的填0，再建立一个64字节（512比特）的数据块，
			//在新的数据块的最后64比特位填文件的比特长度，之前填写0

			memset(p, 0, remain_byte);
			//填满一个数据块就处理
			cal_MD5((uint_32*)_chunk);
			//创建新的数据块,先填0，后面的语句统一填写文件比特长度
			memset(_chunk, 0, CHUNK_BYTE);
		}
		else
		{//如果后面够64比特位（8字节），就直接填充0（后面的语句统一填写文件比特长度）

			memset(p, 0, remain_byte);
		}
		//将文件大小的单位转成bit
		unsigned long long totalbits = _totalByte * 8;
		//强转的目的是：在使用数组的时候，偏移一下偏移8个字节，刚好符合最后的64比特位（8字节）
		((unsigned long long*)_chunk)[7] = totalbits;
		cal_MD5((uint_32*)_chunk);
	}

	//把abcd转成最后的MD5值
	std::string Hex_change_MD5(uint_32 num)
	{
		//通过数字的映射来将字符添加到输出的字符串
		const std::string strMap = "0123456789abcdef";
		std::string ans;
		for (int i = 0; i < 4; ++i)
		{
			//每次循环都右移，获得最后八位的数字
			int tmp = ((num >> (i * 8)) & 0xff);
			//将数字转成字符
			ans += strMap[tmp / 16];
			ans += strMap[tmp % 16];
		}
		return ans;
	}

	std::string getStringMD5(const std::string& str)
	{
		if (str.empty())
		{
			//如果为空，直接返回空的MD5值
			return Hex_change_MD5(_a).append(Hex_change_MD5(_b)).append(Hex_change_MD5(_c)).append(Hex_change_MD5(_d));
		}
		_totalByte = str.size();
		//计算文件的数据块的数量
		uint_32 chunk_nums = _totalByte / CHUNK_BYTE;
		//最后一块不足8字节的数据块
		_lastByte = _totalByte % CHUNK_BYTE;
		const char* p = str.c_str();
		//计算所有完整的数据块
		for (int i = 0; i < (int)chunk_nums; ++i)
		{
			//每次考8个字节（一个数据块）然后计算
			memcpy(_chunk, p + (i*CHUNK_BYTE), CHUNK_BYTE);
			cal_MD5((uint_32*)_chunk);
		}
		//将最后一块拷贝，然后计算
		memcpy(_chunk, p + (CHUNK_BYTE*chunk_nums), _lastByte);
		cal_finalMD5();
		return Hex_change_MD5(_a).append(Hex_change_MD5(_b)).append(Hex_change_MD5(_c)).append(Hex_change_MD5(_d));
	}
	std::string getFileMD5(const char* filePath)
	{
		std::ifstream in_file(filePath, std::ifstream::binary);
		if (!in_file.is_open())
		{
			perror("file open failed!");

		}
		while (!in_file.eof())
		{
			//整体全部读取。以空间换时间
			/*in_file.seekg(0, in_file.end);
			int length = in_file.tellg();
			in_file.seekg(0, in_file.beg);
			char* totalData = new char[length];
			in_file.read(totalData, length);*/
			in_file.read(_chunk, CHUNK_BYTE);
			if (in_file.gcount() != CHUNK_BYTE)
			{
				break;
			}
			_totalByte += CHUNK_BYTE;
			cal_MD5((uint_32*)_chunk);
		}
		_lastByte = in_file.gcount();
		_totalByte += in_file.gcount();
		cal_finalMD5();

		return Hex_change_MD5(_a).append(Hex_change_MD5(_b)).append(Hex_change_MD5(_c)).append(Hex_change_MD5(_d));
	}
private:

	//循环左移的数组
	static int _leftShift[64];
	//_k[i] = floor(2^(32) * abs(sin(i + 1))) 
	uint_32 _k[64];
	//数据块（64byte）
	char _chunk[CHUNK_BYTE];
	//最后一块数据的字节数
	uint_32 _lastByte;
	//总字节数
	long long _totalByte;
	//MD5的buffer
	uint_32 _a;
	uint_32 _b;
	uint_32 _c;
	uint_32 _d;
};
int MD5::_leftShift[64] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 5, 9, 14, 20, 5, 9, 14,
						20, 5, 9, 14, 20, 5, 9, 14, 20, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						4, 11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };