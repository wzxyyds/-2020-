#pragma once

#include "HttpProcessor.h"
#include <sstream>
#include <map>
#include <iostream>
#include <regex>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
#include <windows.h>

#include <winsock2.h>
#include<Ws2tcpip.h>


#include <fstream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")

map<int, string> HttpProcessor::codeToPhrase = { {200, "OK"},
                                                {400, "Bad Request"},
                                                {401, "Unauthorized"},
                                                {403, "Forbidden"},
                                                {404, "Not Found"},
                                                {500, "Internal Server Error"},
                                                {503, "Server Unavailable"} };

/*
 * ����HTTP����
 */
void HttpProcessor::process() {
    //���ղ��洢������
    long recvLen = recv(c_sock, recvMassage, sizeof(recvMassage), 0);
    if (recvLen == 1)
        cout << "!! receive error\n";
    else if (recvLen == 0) {
        cout << "!! disconnect\n";
        return;
    }
    else
        //��ȡ������
        getRequestLine();
    //���ɲ�������Ӧ����
    if (sendResponse()) {
       // cout << "send respond massage successful" << endl;
    }
    else {
        cout << "!! failed to send respond massage: " + to_string(statesCode) +
            " | URL: " + url + "\n";
    }
    //close(c_sock);
    closesocket(c_sock);
}

/*
 * description: ��ȡ������
 * return: 1�ɹ�������0�����쳣
 */
int HttpProcessor::getRequestLine() {
    // ƥ�������е�������ʽ
    smatch sMatch;

    // GET / HTTP/1.1
    // �ο�����         ����ʽ      /index.html �ļ�·��    HTTP    1   .  1  �汾
    regex regulation("([A-Za-z]+) +(.*) +(HTTP/[0-9][.][0-9])");
    

    string s(recvMassage);

    //û��ƥ�䵽
    // ������Υ��
    if (!regex_search(s, sMatch, regulation)) {

        statesCode = 400;
        cout << "!! request massage format exception\n";
        return 0;
    }


    // ����URL
    char clientIP[20];

    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, 20);


    cout << "-- " + sMatch[0].str() + " | from IP:" + string(clientIP) +
        " ; port:" + to_string(clientAddr.sin_port) + "\n";

    this->method = sMatch[1];  //����ʽ

    cout << "sMatch[2]" << endl;
    cout << sMatch[2].str() << endl;
    //cout << sMatch[2].str().size() << endl;
    //cout << (sMatch[2].str().size() == sMatch[2].str().length()) << endl;

    string str = sMatch[2].str();
    if (str == "/") {
        this->url = "/index.html";
    }
    else {
        this->url = str.substr(0, str.find('?'));
    }
    return 1;
}

/*
 * description: ���ɲ�������Ӧ����
 * return: 1��ȷ��Ӧ����0�쳣
 */
int HttpProcessor::sendResponse() {
    //ifstream f(file_path + url, ios::in);
    ifstream f(file_path + url, ios::binary);


    cout << "������ļ�·��Ϊ��" << (file_path + url) << endl;
    cout << file_path << endl;
    cout << url << endl;

    // ���û�ҵ��ļ�
    if (!f.is_open()) {
        statesCode = 404;

        cout << "������ļ������ڣ�\n" << endl;

        //���ļ� "./files/404.html" 
        //f.open(NOT_FOUND_FILE, ios::in);
        f.open(NOT_FOUND_FILE, ios::binary);

        if (!f.is_open()) {
            cout << "!! not open 404 file\n";
        }
        cout << "�Ѵ�404�ļ�\n" << endl;
    }

    std::string Content_Type;

    //������ļ�����
    string file_suffix = url.substr(url.find('.') + 1);
    if (file_suffix == "jpg" || file_suffix == "ico") {
        //cout << "this is a image" << endl;
        Content_Type = "image/jpg";
    }
    else if (file_suffix == "gif") {
        Content_Type = "image/gif";
    }
    else {
        Content_Type = "text/html";
    }

    //���ļ�����������
    filebuf* temp = f.rdbuf();
    int size = temp->pubseekoff(0, f.end, f.in);
    temp->pubseekpos(0, f.in);

    char* buffer = new char[size];
    char* tail = buffer + size;
    temp->sgetn(buffer, size);
    f.close();
    cout << url << " return success" << endl << endl;
    stringstream remsg;
    remsg << "HTTP/1.1 200 OK\r\n" << "Connection:close\r\n" << "Server:Macyrate\r\n" << "Content Length:" << size
        << "\r\n" << "Content Type:" + Content_Type << "\r\n\r\n";
    string remsgstr = remsg.str();
    const char* remsgchar = remsgstr.c_str();
    int tmpsize = strlen(remsgchar);
    int sendre = send(c_sock, remsgchar, tmpsize, 0);
    while (buffer < tail) {
        sendre = send(c_sock, buffer, size, 0);
        buffer = buffer + sendre;
        size = size - sendre;
    }
    return 1;
}