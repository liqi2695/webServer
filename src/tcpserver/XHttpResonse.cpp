#include "XHttpResonse.h"
#include<string>
#include<regex>
#include<iostream>
#define _CRT_SECURE_NO_WARNINGS


using namespace std;
//�����ļ���С
bool XHttpResonse::SetRequest(std::string request)
{
	string src = request;
	// /  /index.html  //ff/index.php 
	string pattern = "^([A-Z]+) /([a-zA-Z0-9]*([.][a-zA-Z]*)?)[?]?(.*) HTTP/1";
	regex r(pattern);
	smatch mas;
	regex_search(src, mas, r);
	if (mas.size() == 0)
	{
		cout << pattern << " failed" << endl;
		return false;
	}
	//ȡ����·�����
	string type = mas[1];//GET
	string path = "/";
	path += mas[2];//index.html
	string filetype = mas[3];
	string query = mas[4];

	if (type != "GET")
	{
		cout << "Not GET!!!" << endl;

		return false;

	}
	string filename = path;
	if (path == "/")
	{
		filename = "/index.html";
	}
	string filepath = "www";
	filepath += filename;

	if (filetype == "php")
	{
		string cmd = "php-cgi ";
		cmd += filepath;
		cmd += " ";
		//query id=1&name=xcj
		//id = 1 name = xcj
		for (int i = 0; i < query.size(); i++)
		{
			if (query[i] == '&') query[i] = ' ';
		}
		cmd += query;

		cmd += " > ";
		filepath += ".html";
		cmd += filepath;
		//����������˼����php-cgi www/index.php  > www/index.php.html
		printf("%s\n", cmd.c_str());
		system(cmd.c_str());//C�����Դ�����������ֱ��ִ��ָ��
	}

	fp = fopen(filepath.c_str(), "rb");
	if (fp == NULL)
	{
		cout << "open file failed" << filetype.c_str() << endl;
		return false;
	}

	//��ȡ�ļ���С
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);//�����int�Ĵ�С����
	fseek(fp, 0, 0);
	cout << "file size is:" << filesize << endl;

	//����ͷ
	if (filetype == "php")
		//ֻ����php�ļ��Ž���ͷ������Ӱ��������̬�ļ��ķ���
	{
		char c = 0;
		//\r\n\r\n
		int headsize = 0;
		while (fread(&c, 1, 1, fp) > 0)
		{
			headsize++;
			if (c == '\r')
			{
				fseek(fp, 3, SEEK_CUR);
				//����/r����ƶ���λ��Ȼ��͵�������
				headsize += 3;
				break;
			}
		}
		filesize = filesize - headsize;//��filesize��ֵ���иı�
	}

	return true;
}
//��Ӧ����������Ϣͷ������ȡͷ��Ϣ
std::string XHttpResonse::GetHead()
{
	//��Ӧhttp��GET����
    //��Ϣͷ
	string rmsg = "";
	rmsg = "HTTP/1.1 200 OK\r\n"; //Э��
	rmsg += "Server:XHttp\r\n";//��\r\rn����
	rmsg += "Content-Type: text/html\r\n";
	rmsg += "Content-Length: ";
	char bsize[128] = { 0 };

	sprintf(bsize, "%d", filesize);//************



	
	rmsg += bsize;
	//rmsg += "10\r\n";//���Ʒ��͵Ĵ�С 
	rmsg += "\r\n\r\n";
	//��Ϣ����
	//rmsg += "0123456789";
	//������Ϣͷ
	return rmsg;
}

//����
int XHttpResonse::Read(char* buf, int bufsize)
{
	//��������
	return  fread(buf, 1, bufsize, fp);
}