#pragma once
#include "stdafx.h"

class TCPServerBootstrap 
{
public:
	const int DefaultAcceptCount = 10;
public:
	//构造函数
	TCPServerBootstrap();
	//虚构函数
	~TCPServerBootstrap();
public:
	//开始监听
	bool StartListening(unsigned short port, const std::string& ip = "0.0.0.0");
	void GetConnectionClient();
	/*
	释放3个部分步骤：
	1. 清空IOCP线程队列，退出线程
	2. 清空等待accept套接字m_vecAcps
	3. 清空已连接套接字m_vecContInfo并清空缓存
	*/
	void CloseServer();
protected:
	//处理recv请求
	virtual bool DoRecv(COverlappedIOInfo* info);
	//处理recv请求
	virtual bool DoAfterSend(COverlappedIOInfo* info);
	//私有成员函数
private:
	//启动CPU*2个线程，返回已启动的线程个数
	size_t StartWorkThreadPool();
	//获取AcceptEx_和 GetAcceptExSockaddrs 函数指针
	bool GetLPFNAcceptExAndGetAcceptSockaddrs();
	//利用AcceptEx_监听accept请求
	bool PostAccept(COverlappedIOInfo* info);
	//处理accept请求， NumberOfBytes=0表示没有收到第一帧数据， >0时表示收到了第一帧数据
	bool DoAccept(COverlappedIOInfo* info, DWORD NumberOfBytes = 0);
	//投递recv请求
	bool PostRecv(COverlappedIOInfo* info);
	//从已连接的socket列表中移除socket及释放空间
	bool DeleteLink(SOCKET s);

	//私有成员变量
private:
	//winsock版本类型
	WSAData wsaData;
	//端口监听套接字
	SOCKET listeningSocket;
	//等待accept的套接字，这些套接字都是没有使用过的，数量为ACCEPT_SOCKET_NUM。同时会有10个套接字等待accept
	std::vector<SOCKET> accepts;
	//已建立连接的信息，每个结构含有一个套接字、发送缓冲区、接收缓冲区、对端地址
	std::vector<COverlappedIOInfo*> conInfos;
	//操作vector的互斥访问锁
	std::mutex _mutex;
	//CIOCP封装类
	CIOCP iocp;
	//AcceptEx_函数指针
	LPFN_ACCEPTEX acceptExFunction;
	//GetAcceptSockAddrs函数指针
	LPFN_GETACCEPTEXSOCKADDRS getAcceptSocketFunction;

	//工作线程池
	std::vector<std::thread> pool;
	//工作线程池大小
	size_t poolSize;
};

typedef Singleton<TCPServerBootstrap> BasicTCPServer;
