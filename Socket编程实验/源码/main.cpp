#pragma once
#include "HttpProcessor.h"
#include <thread>
#include <fstream>
#include <iostream>

#include <string.h> //���bzero()

#include <winsock2.h>
#include<Ws2tcpip.h>

using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define MAXLINK 1024

string filePath; //��������Դ·��
string version;  //�汾��
string IP;       // IP��ַ
int port;        //�˿ں�

void catchOpt();
void config();
bool createConfigFile();

/*
 * ������Ʋ���
 */
void catchOpt() {
    while (int ch = getchar()) {
        //�˳�����
        if (ch == 'q') {
            exit(0);
        }
        //��������
        if (ch == 'c') {
            config();
        }
    }
}

/*
 * ���ļ��ж�ȡ���ã�����ļ��𻵻��ʽ���������´��������ļ�
 */
void config() {
    ifstream config("config", ios::in);
    if (!config.is_open()) {
        cout << "config file is missing" << endl;
        if (createConfigFile())
            return;
    }
    char tmp[128] = {};
    string info;
    config.getline(tmp, sizeof(tmp));
    info = string(tmp);
    //���汾��Ϣ
    if (info.substr(0, info.find(':')) != "version") {
        cout << "config file is broken" << endl;
        if (createConfigFile())
            return;
    }
    else {
        version = info.substr(info.find(':') + 1);
    }
    //����Դ·��
    config.getline(tmp, sizeof(tmp));
    info = string(tmp);
    if (info.substr(0, info.find(':')) != "filePath") {
        cout << "config file is broken" << endl;
        if (createConfigFile())
            return;
    }
    else {
        filePath = info.substr(info.find(':') + 1);
    }
    //��IP��ַ
    config.getline(tmp, sizeof(tmp));
    info = string(tmp);
    if (info.substr(0, info.find(':')) != "IP") {
        cout << "config file is broken" << endl;
        if (createConfigFile())
            return;
    }
    else {
        IP = info.substr(info.find(':') + 1);
    }

    //���˿ں�
    config.getline(tmp, sizeof(tmp));
    info = string(tmp);
    if (info.substr(0, info.find(':')) != "port") {
        cout << "config file is broken" << endl;
        if (createConfigFile())
            return;
    }
    else {
        string p = (info.substr(info.find(':') + 1));
        port = stoi(p);
    }
    cout << "----configurations renewed successfully----" << endl;
    cout << "- version: " << version << endl;
    cout << "- filePath: " << filePath << endl;
    cout << "- IP: " << IP << endl;
    cout << "- port: " << port << endl;
    cout << "----new configuration will apply to later connections----" << endl;
}

/*
 * ���´��������ļ�
 */
bool createConfigFile() {
    cout << "now you need to offer following information to recreate "
        "configuration file:"
        << endl;
    cout << "- version: ";
    cin >> version;
    cout << "- filePath: ";
    cin >> filePath;
    cout << "- IP: ";
    cin >> IP;
    cout << "- port: ";
    cin >> port;
    cout << "----configurations renewed successfully----" << endl;
    cout << "----new configuration will apply to later connections----" << endl;

    //�������û��config�ļ����Զ������ٴ�����
    ofstream config("config", ios::out);
    if (!config.is_open()) {
        cout << "fail to create config file" << endl;
        return false;
    }
    config << "version:" << version << endl
        << "filePath:" << filePath << endl
        << "IP:" << IP << endl
        << "port:" << port << endl;
    config.close();
    cout << "create config file successfully" << endl;
    return true;
}

int main() {
    WSADATA wsaData;
    int nRc = WSAStartup(0x0202, &wsaData);

    if (nRc) {
        printf("Winsock  startup failed with error!\n");
    }

    if (wsaData.wVersion != 0x0202) {
        printf("Winsock version is not correct!\n");
    }

    printf("Winsock  startup Ok!\n");

    //////////////////////

    //���÷���������
    config();


    //���������׽���
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == -1) {
        perror("! socket error\n");
        return 1;
    }

    //�󶨽�����������Ķ˿�
    sockaddr_in _sin = {};

    //bzero(&_sin, sizeof(_sin));
    memset(&_sin, 0, sizeof(_sin));


    _sin.sin_family = AF_INET;   //Э����IPV4
    _sin.sin_port = htons(port); //�˿ں� host_to_net

    if (IP.empty() || IP == "-" || IP == "all") {
        _sin.sin_addr.s_addr = htonl(INADDR_ANY); //���޶����ʸ÷�������ip
    }
    else {
        inet_pton(AF_INET, IP.data(), &_sin.sin_addr);
    }
    if (-1 == ::bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
        cout << "!! bind error" << endl;
        return 0;
    }

    //�����˿�
    if (listen(_sock, MAXLINK) == -1) {
        perror("!! listen error\n");
    }
    else {
        cout << ">>> listening" << endl;
    }

    thread catchOp(catchOpt);
    catchOp.detach();

    while (true) {
        //�ȴ���������
        sockaddr_in clientAddr = {};
        socklen_t nAddrLen = sizeof(sockaddr_in);
        int _cSock =
            accept(_sock, (sockaddr*)&clientAddr,
                &nAddrLen); //�׽��֣��յ��ͻ���socket��ַ������socket��ַ�Ĵ�С
        if (_cSock < 0) {
            perror("!! client socket error!\n");
        }

        HttpProcessor* httpProcessor =
            new HttpProcessor(version, filePath, _cSock, clientAddr);
        //��������
        // httpProcessor->process();//���߳�ʵ��

        //���߳�ʵ��
        thread process(&HttpProcessor::process, httpProcessor);

        //����һ��������ַ��һ������ĵ�ַ
        //�ѵ���Ϊ����ĺ�������������ͨ����ô��

        process.detach();
    }
}
