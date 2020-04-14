#pragma once
#include<string>
#include<utility>
#include"httplib.h"
#include"File_Info_Manager.hpp"

class CloudBackupClient
{
private:
	std::string _svr_ip;
	uint16_t _svr_port;
	FileInfoManager _fim;
	std::string _listen_dir;//在磁盘中的文件夹（要备份的文件就放里面）
public:
	CloudBackupClient(const std::string& listen_file, const std::string& file_info_file, const std::string& svr_ip, uint16_t svr_port)
		:_listen_dir(listen_file)
		, _fim(file_info_file)
		, _svr_ip(svr_ip)
		, _svr_port(svr_port)
	{
		//若文件夹不存在则创建
		if (boost::filesystem::exists(_listen_dir) == false)
		{
			boost::filesystem::create_directory(_listen_dir);
		}
	}

	void Start()
	{
		//将客户端连接服务端
		httplib::Client client(_svr_ip.c_str(), _svr_port);
		while (1)
		{
			printf("开始云备份... ...\n");
			//一、先获取需要压缩的文件列表
			std::unordered_map<std::string, std::string> list;
			GetNeedBackupList(&list, _listen_dir);
			//二、将所有要上传的文件上传
			for (const auto& file : list)
			{
				//1、拿到文件名和带路径的文件名
				//文件名是为了用于request
				//带路径的文件名是为了读取文件数据
				std::string path_file_name = file.first;
				std::string file_name = file.second;
				printf(" %s is need to backup ...\n", file_name.c_str());
				//2、将文件数据读出
				std::string buf;
				FileTool::Read(path_file_name, &buf);
				printf("buf的大小是：%d\n", buf.size());
				//3、发送数据
				std::string request_path = "/" + file_name;
				auto rsp = client.Put(request_path.c_str(), buf, "application/octet-stream");//以二进制流文件下载
				if (rsp == NULL || (rsp->status != 200))
				{
					printf(" %s backup failed!\n", file_name.c_str());
					continue;
				}
				//4、更新文件信息列表
				std::string file_md5;
				FileTool::CalFileMd5(path_file_name, &file_md5);
				_fim.Insert(file_name, file_md5);
				printf(" %s backup success!\n", file_name.c_str());
			}
			printf("此次扫描完毕... ...\n");
			Sleep(10000);
		}
	}
private:
	void GetNeedBackupList(std::unordered_map<std::string,std::string>* list, std::string file_path)
	{
		//扫描文件夹下哪些文件需要备份：
		//1、若文件夹里的某些文件 在 存文件信息的文件里没有找到，表示这个文件是新加进去的
		//2、若文件夹里的某个文件的md5值 和 对应存文件信息的文件的md5值不一样，表示更新过
		printf("在扫盘... ...\n");
		boost::filesystem::directory_iterator begin(file_path);
		boost::filesystem::directory_iterator end;
		for (; begin != end; ++begin)
		{
			//如果是文件夹则进入文件夹
			if (boost::filesystem::is_directory(begin->status()) == true)
			{
				//如果是当前文件或者上级文件则跳过
				if (begin->path().string() != "." || begin->path().string() != "..")
				{
					//如果是普通文件夹则递归的进入这个函数继续扫描，直到扫描到文件
					std::string depth_path = begin->path().string();
					GetNeedBackupList(list, depth_path);
				}
			}
			else 
			{
				//a.先计算这个文件的md5值
				std::string new_md5;
				std::string path_file_name = begin->path().string();
				FileTool::CalFileMd5(path_file_name, &new_md5);
				//b.在拿到之前（最后一次上传的时候）这个文件的md5值
				std::string old_md5;
				std::string file_name = begin->path().filename().string();
				_fim.GetMd5(file_name, &old_md5);
				//比较两个md5，不同则需要备份
				//ps：如果_fim.GetMd5(file_name, &old_md5);里找不到，则old_md5会是空，则表示new_md5是新放进去的文件
				if (old_md5 != new_md5)
				{
					list->insert(std::make_pair(path_file_name, file_name));
				}
			}
		}
	}
};