#pragma once
/*
在IOCP编程模型中，需要用到GetQueuedCompletionStatus()函数获取已完成的事件
但是该函数的返回参数无 Socket或者buffer的描述信息

一个简单的方法是：创建一个新的结构，该结构第一个参数是OVERLAPPED
由于AcceptEx, WSASend 等重叠IO操作传入的是Overlapped结构体的地址，调用AcceptEx等重叠IO操作，在Overlapped结构体后面开辟新的空间
写入socket或者buffer的信息，即可将socket或者buffer的信息由GetQueuedComletionStatus带回
*/
#include "stdafx.h"
#define MAXBUF 8*1024
//MTU:46~1500

enum class IOCP_OperationType :int
{
	TYPE_ACP,					//accept事件到达，有新的连接请求
	TYPE_RECV,				   //数据接收事件
	TYPE_SEND,               //数据发送事件
	TYPE_CLOSE,             //关闭事件
	TYPE_NO_OPER  
};

class COverlappedIOInfo 
	: public OVERLAPPED
{
public:
	COverlappedIOInfo(void)
		: socket(INVALID_SOCKET)
		, recvBuf()
		, crecvBuf()
		, sendBuf()
		, csendBuf()
		, peerAddress()
	{
		ResetOverlapped();
		ResetRecvBuffer();
		ResetSendBuffer();
	}
	~COverlappedIOInfo(void)
	{
		if (socket != INVALID_SOCKET)
		{
			closesocket(socket);
			socket = INVALID_SOCKET;
		}
	}

	void ResetOverlapped()
	{
		Internal = InternalHigh = 0;
		Offset = OffsetHigh = 0;
		hEvent = NULL;
	}
	void ResetRecvBuffer()
	{
		ZeroMemory(crecvBuf, MAXBUF);
		recvBuf.buf = crecvBuf;
		recvBuf.len = MAXBUF;
	}
	void ResetSendBuffer()
	{
		ZeroMemory(csendBuf, MAXBUF);
		sendBuf.buf = csendBuf;
		sendBuf.len = MAXBUF;
	}


public:
	SOCKET socket;		//套接字

	//接收缓冲区，用于AcceptEx, WSARecv操作
	WSABUF recvBuf;	
	char crecvBuf[MAXBUF];

	//发送缓冲区， 用于WSASend操作
	WSABUF sendBuf;  
	char csendBuf[MAXBUF];

	//对端地址
	sockaddr_in peerAddress;
};