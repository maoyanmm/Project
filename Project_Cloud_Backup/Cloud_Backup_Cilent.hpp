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
	std::string _listen_dir;//�ڴ����е��ļ��У�Ҫ���ݵ��ļ��ͷ����棩
public:
	CloudBackupClient(const std::string& listen_file, const std::string& file_info_file, const std::string& svr_ip, uint16_t svr_port)
		:_listen_dir(listen_file)
		, _fim(file_info_file)
		, _svr_ip(svr_ip)
		, _svr_port(svr_port)
	{
		//���ļ��в������򴴽�
		if (boost::filesystem::exists(_listen_dir) == false)
		{
			boost::filesystem::create_directory(_listen_dir);
		}
	}

	void Start()
	{
		//���ͻ������ӷ����
		httplib::Client client(_svr_ip.c_str(), _svr_port);
		while (1)
		{
			printf("��ʼ�Ʊ���... ...\n");
			//һ���Ȼ�ȡ��Ҫѹ�����ļ��б�
			std::unordered_map<std::string, std::string> list;
			GetNeedBackupList(&list, _listen_dir);
			//����������Ҫ�ϴ����ļ��ϴ�
			for (const auto& file : list)
			{
				//1���õ��ļ����ʹ�·�����ļ���
				//�ļ�����Ϊ������request
				//��·�����ļ�����Ϊ�˶�ȡ�ļ�����
				std::string path_file_name = file.first;
				std::string file_name = file.second;
				printf(" %s is need to backup ...\n", file_name.c_str());
				//2�����ļ����ݶ���
				std::string buf;
				FileTool::Read(path_file_name, &buf);
				printf("buf�Ĵ�С�ǣ�%d\n", buf.size());
				//3����������
				std::string request_path = "/" + file_name;
				auto rsp = client.Put(request_path.c_str(), buf, "application/octet-stream");//�Զ��������ļ�����
				if (rsp == NULL || (rsp->status != 200))
				{
					printf(" %s backup failed!\n", file_name.c_str());
					continue;
				}
				//4�������ļ���Ϣ�б�
				std::string file_md5;
				FileTool::CalFileMd5(path_file_name, &file_md5);
				_fim.Insert(file_name, file_md5);
				printf(" %s backup success!\n", file_name.c_str());
			}
			printf("�˴�ɨ�����... ...\n");
			Sleep(10000);
		}
	}
private:
	void GetNeedBackupList(std::unordered_map<std::string,std::string>* list, std::string file_path)
	{
		//ɨ���ļ�������Щ�ļ���Ҫ���ݣ�
		//1�����ļ������ĳЩ�ļ� �� ���ļ���Ϣ���ļ���û���ҵ�����ʾ����ļ����¼ӽ�ȥ��
		//2�����ļ������ĳ���ļ���md5ֵ �� ��Ӧ���ļ���Ϣ���ļ���md5ֵ��һ������ʾ���¹�
		printf("��ɨ��... ...\n");
		boost::filesystem::directory_iterator begin(file_path);
		boost::filesystem::directory_iterator end;
		for (; begin != end; ++begin)
		{
			//������ļ���������ļ���
			if (boost::filesystem::is_directory(begin->status()) == true)
			{
				//����ǵ�ǰ�ļ������ϼ��ļ�������
				if (begin->path().string() != "." || begin->path().string() != "..")
				{
					//�������ͨ�ļ�����ݹ�Ľ��������������ɨ�裬ֱ��ɨ�赽�ļ�
					std::string depth_path = begin->path().string();
					GetNeedBackupList(list, depth_path);
				}
			}
			else 
			{
				//a.�ȼ�������ļ���md5ֵ
				std::string new_md5;
				std::string path_file_name = begin->path().string();
				FileTool::CalFileMd5(path_file_name, &new_md5);
				//b.���õ�֮ǰ�����һ���ϴ���ʱ������ļ���md5ֵ
				std::string old_md5;
				std::string file_name = begin->path().filename().string();
				_fim.GetMd5(file_name, &old_md5);
				//�Ƚ�����md5����ͬ����Ҫ����
				//ps�����_fim.GetMd5(file_name, &old_md5);���Ҳ�������old_md5���ǿգ����ʾnew_md5���·Ž�ȥ���ļ�
				if (old_md5 != new_md5)
				{
					list->insert(std::make_pair(path_file_name, file_name));
				}
			}
		}
	}
};