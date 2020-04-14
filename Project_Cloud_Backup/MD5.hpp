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

	//��ʼ����_k[64],�͵���reset����
	void init()
	{
		// ��Ϊ�˴�i��0��ʼ��������Ҫsin(i + 1)
		for (int i = 0; i < 64; ++i)
		{
			_k[i] = (uint_32)(abs(sin(i + 1.0))*pow(2.0, 32));
		}
		reset();
	}

	//����:_a,_b,_c,_d
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

	//λ���㺯�� F G H I
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

	//ѭ������
	uint_32 left_shift(uint_32 number, int shiftNumber)
	{
		return ((number << shiftNumber) | (number >> (32 - shiftNumber)));
	}

	//����һ�����ݿ�chunk��MD5ֵ
	void cal_MD5(uint_32* chunk)
	{
		int a = _a;
		int b = _b;
		int c = _c;
		int d = _d;
		int tmp_4;
		int g;
		//��������ݿ�һ������4�֣�ÿ�ִ���16�Σ�ÿ�δ����ĸ��ֽ�
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
		//�����ĸ�ֵ
		_a += a;
		_b += b;
		_c += c;
		_d += d;
	}

	//�����һ��������������λ�ͱ���λ����
	void cal_finalMD5()
	{
		char* p = _chunk + _lastByte;
		//���������Ϣ���������һ���ֽڣ���Ϊ�ļ�����С��λ��һ���ֽڣ�
		*p++ = (char)0x80;
		int remain_byte = CHUNK_BYTE - _lastByte - 1;
		if (remain_byte < 8)
		{//������治��64����λ��8�ֽڣ����Ͱ�ʣ�����0���ٽ���һ��64�ֽڣ�512���أ������ݿ飬
			//���µ����ݿ�����64����λ���ļ��ı��س��ȣ�֮ǰ��д0

			memset(p, 0, remain_byte);
			//����һ�����ݿ�ʹ���
			cal_MD5((uint_32*)_chunk);
			//�����µ����ݿ�,����0����������ͳһ��д�ļ����س���
			memset(_chunk, 0, CHUNK_BYTE);
		}
		else
		{//������湻64����λ��8�ֽڣ�����ֱ�����0����������ͳһ��д�ļ����س��ȣ�

			memset(p, 0, remain_byte);
		}
		//���ļ���С�ĵ�λת��bit
		unsigned long long totalbits = _totalByte * 8;
		//ǿת��Ŀ���ǣ���ʹ�������ʱ��ƫ��һ��ƫ��8���ֽڣ��պ÷�������64����λ��8�ֽڣ�
		((unsigned long long*)_chunk)[7] = totalbits;
		cal_MD5((uint_32*)_chunk);
	}

	//��abcdת������MD5ֵ
	std::string Hex_change_MD5(uint_32 num)
	{
		//ͨ�����ֵ�ӳ�������ַ���ӵ�������ַ���
		const std::string strMap = "0123456789abcdef";
		std::string ans;
		for (int i = 0; i < 4; ++i)
		{
			//ÿ��ѭ�������ƣ��������λ������
			int tmp = ((num >> (i * 8)) & 0xff);
			//������ת���ַ�
			ans += strMap[tmp / 16];
			ans += strMap[tmp % 16];
		}
		return ans;
	}

	std::string getStringMD5(const std::string& str)
	{
		if (str.empty())
		{
			//���Ϊ�գ�ֱ�ӷ��ؿյ�MD5ֵ
			return Hex_change_MD5(_a).append(Hex_change_MD5(_b)).append(Hex_change_MD5(_c)).append(Hex_change_MD5(_d));
		}
		_totalByte = str.size();
		//�����ļ������ݿ������
		uint_32 chunk_nums = _totalByte / CHUNK_BYTE;
		//���һ�鲻��8�ֽڵ����ݿ�
		_lastByte = _totalByte % CHUNK_BYTE;
		const char* p = str.c_str();
		//�����������������ݿ�
		for (int i = 0; i < (int)chunk_nums; ++i)
		{
			//ÿ�ο�8���ֽڣ�һ�����ݿ飩Ȼ�����
			memcpy(_chunk, p + (i*CHUNK_BYTE), CHUNK_BYTE);
			cal_MD5((uint_32*)_chunk);
		}
		//�����һ�鿽����Ȼ�����
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
			//����ȫ����ȡ���Կռ任ʱ��
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

	//ѭ�����Ƶ�����
	static int _leftShift[64];
	//_k[i] = floor(2^(32) * abs(sin(i + 1))) 
	uint_32 _k[64];
	//���ݿ飨64byte��
	char _chunk[CHUNK_BYTE];
	//���һ�����ݵ��ֽ���
	uint_32 _lastByte;
	//���ֽ���
	long long _totalByte;
	//MD5��buffer
	uint_32 _a;
	uint_32 _b;
	uint_32 _c;
	uint_32 _d;
};
int MD5::_leftShift[64] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 5, 9, 14, 20, 5, 9, 14,
						20, 5, 9, 14, 20, 5, 9, 14, 20, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						4, 11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };