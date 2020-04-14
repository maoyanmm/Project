#pragma once
#include<string>
#include<fstream>
#include<boost/filesystem.hpp>
#include"MD5.hpp"

//文件的工具类（读，写）
class FileTool
{
public:
	//从文件中读取内容
	static bool Read(const std::string& name, std::string* body)
	{
		std::ifstream ifs(name, std::ios::binary);
		if (ifs.is_open() == false)
		{
			printf("file %s open failed!\n", name.c_str());
			return false;
		}
		//得到该文件的大小
		int64_t fsize = boost::filesystem::file_size(name);
		printf("这个[%s]文件的大小是:.............%d\n", name.c_str(),fsize);
		//将要接受的容器扩容
		body->resize(fsize);
		ifs.read(&((*body)[0]), fsize);
		//判断上次操作是否成功
		if (ifs.good() == false)
		{
			printf("file %s read failed!\n", name.c_str());
			return false;
		}
		ifs.close();
		return true;
	}
	static bool Write(const std::string& name, const std::string& body)
	{
		std::ofstream ofs(name, std::ios::binary);
		if (ofs.is_open() == false)
		{
			printf("Write : file %s open failed!\n", name.c_str());
			return false;
		}
		ofs.write(&body[0], body.size());
		if (ofs.good() == false)
		{
			printf("file %s write failed!\n", name.c_str());
			return false;
		}
		ofs.close();
		return true;
	}
	static void CalFileMd5(const std::string& path_file_name, std::string* file_md5)
	{
		MD5 md5;
		*file_md5 = md5.getFileMD5(path_file_name.c_str());
	}
};