#pragma once
#include<string>
#include<unordered_map>
#include<vector>
#include<sstream>
#include<boost/algorithm/string.hpp>

#include"FIleTool.hpp"

class FileInfoManager
{
private:
	std::string _store_file;//磁盘里存储文件信息的文件
	std::unordered_map<std::string, std::string> _file_info_list;//备份文件信息的容器
public:
	FileInfoManager(const std::string& file_name)
		:_store_file(file_name)
	{
		InitLoad();
	}
	//将文件信息存储到磁盘文件里
	void Storage()
	{
		std::stringstream ss;
		for (auto& file_info : _file_info_list)
		{
			//各个文件间用/r/n来分隔，每一个文件的 文件名和md5值 用空格分隔
			ss << file_info.first << " " << file_info.second << "\r\n";
		}
		FileTool::Write(_store_file, ss.str());

	}
	//将磁盘文件里读取文件信息到内存
	bool InitLoad()
	{
		//1、先读出文件内容
		std::string buf;
		if (FileTool::Read(_store_file, &buf) == false)
		{
			return false;
		}
		//2、用boost库来分割内容
		//a.先分割各个文件
		std::vector<std::string> list;
		boost::algorithm::split(list, buf, boost::is_any_of("\r\n"), boost::token_compress_off);
		//b.在list里分割文件名和MD5
		for (const auto& file_info : list)
		{
			size_t pos = file_info.find(" ");
			std::string file_name = file_info.substr(0, pos);
			//pos+1是因为空格不算
			std::string file_md5 = file_info.substr(pos + 1);
			//将文件名和md5值读到列表里
			_file_info_list[file_name] = file_md5;
		}
		return true;
	}
	//拿到一个文件的md5值
	bool GetMd5(const std::string& file_name,std::string* file_md5)
	{
		auto it = _file_info_list.find(file_name);
		if (it == _file_info_list.end())
		{
			return false;
		}
		*file_md5 = it->second;
		return true;
	}
	//插入/更新一个文件信息
	void Insert(const std::string& file_name, const std::string& file_md5)
	{
		_file_info_list[file_name] = file_md5;
		Storage();
	}
};