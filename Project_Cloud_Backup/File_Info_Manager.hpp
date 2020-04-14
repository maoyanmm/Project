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
	std::string _store_file;//������洢�ļ���Ϣ���ļ�
	std::unordered_map<std::string, std::string> _file_info_list;//�����ļ���Ϣ������
public:
	FileInfoManager(const std::string& file_name)
		:_store_file(file_name)
	{
		InitLoad();
	}
	//���ļ���Ϣ�洢�������ļ���
	void Storage()
	{
		std::stringstream ss;
		for (auto& file_info : _file_info_list)
		{
			//�����ļ�����/r/n���ָ���ÿһ���ļ��� �ļ�����md5ֵ �ÿո�ָ�
			ss << file_info.first << " " << file_info.second << "\r\n";
		}
		FileTool::Write(_store_file, ss.str());

	}
	//�������ļ����ȡ�ļ���Ϣ���ڴ�
	bool InitLoad()
	{
		//1���ȶ����ļ�����
		std::string buf;
		if (FileTool::Read(_store_file, &buf) == false)
		{
			return false;
		}
		//2����boost�����ָ�����
		//a.�ȷָ�����ļ�
		std::vector<std::string> list;
		boost::algorithm::split(list, buf, boost::is_any_of("\r\n"), boost::token_compress_off);
		//b.��list��ָ��ļ�����MD5
		for (const auto& file_info : list)
		{
			size_t pos = file_info.find(" ");
			std::string file_name = file_info.substr(0, pos);
			//pos+1����Ϊ�ո���
			std::string file_md5 = file_info.substr(pos + 1);
			//���ļ�����md5ֵ�����б���
			_file_info_list[file_name] = file_md5;
		}
		return true;
	}
	//�õ�һ���ļ���md5ֵ
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
	//����/����һ���ļ���Ϣ
	void Insert(const std::string& file_name, const std::string& file_md5)
	{
		_file_info_list[file_name] = file_md5;
		Storage();
	}
};