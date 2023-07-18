#pragma once
#ifndef WEBSERVER_HTTPPROCESSOR_H
#define WEBSERVER_HTTPPROCESSOR_H

//#include <arpa/inet.h>
//#include <cstdlib>
//#include <fstream>
//#include <iostream>
//#include <map>
//#include <netinet/in.h>
//#include <regex>
//#include <string>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <utility>

#pragma once
#include "winsock2.h"
#include <string>

#include <map>

//#define NOT_FOUND_FILE "../404.html" // 404����ʱ���ص�html�ļ������·��
#define NOT_FOUND_FILE "./files/404.html" // 404����ʱ���ص�html�ļ������·��


using namespace std;

#pragma comment(lib,"ws2_32.lib")

class HttpProcessor {
public:
    /*
     * ����HTTP����
     */
    void process();

    //���캯��
    HttpProcessor(string version, string filePath, SOCKET cSock,
        const sockaddr_in& clientAddr)
        : version(std::move(version)), file_path(std::move(filePath)),
        c_sock(cSock), clientAddr(clientAddr) {}

private:
    //�汾��
    string version;

    //��������Դ�ļ���·��
    string file_path;

    //�����ĵķ���
    string method;

    //������Դ��URL
    string url;

    //�׽���
    SOCKET c_sock;

    //�ͻ��˵�ַ
    sockaddr_in clientAddr;

    //״̬��
    int statesCode = 200;

    //������
    char recvMassage[2048]{};

    //״̬���������Ķ�Ӧ��
    static map<int, string> codeToPhrase;

    /*
     * description: ��ȡ������
     * return: 1�ɹ�������0�����쳣
     */
    int getRequestLine();

    /*
     * description: ���ɲ�������Ӧ����
     * return: 1��ȷ��Ӧ����0�쳣
     */
    int sendResponse();
};

#endif // WEBSERVER_HTTPPROCESSOR_H
