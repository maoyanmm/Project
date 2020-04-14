#include"Cloud_Backup_Cilent.hpp"
#include<iostream>

#define FILE_NAME_DIR "./fileNameList.txt"
#define LISTEN_DIR "./backup"
#define SERVER_IP "192.168.132.128"
#define SERVER_PORT 4418

int main()
{ 
	CloudBackupClient cbc(LISTEN_DIR, FILE_NAME_DIR, SERVER_IP, SERVER_PORT);
	cbc.Start();
	system("pause");
	return 0;
}