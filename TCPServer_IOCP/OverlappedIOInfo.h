#pragma once
/*
��IOCP���ģ���У���Ҫ�õ�GetQueuedCompletionStatus()������ȡ����ɵ��¼�
���Ǹú����ķ��ز����� Socket����buffer��������Ϣ

һ���򵥵ķ����ǣ�����һ���µĽṹ���ýṹ��һ��������OVERLAPPED
����AcceptEx, WSASend ���ص�IO�����������Overlapped�ṹ��ĵ�ַ������AcceptEx���ص�IO��������Overlapped�ṹ����濪���µĿռ�
д��socket����buffer����Ϣ�����ɽ�socket����buffer����Ϣ��GetQueuedComletionStatus����
*/
#include "stdafx.h"
#define MAXBUF 8*1024
//MTU:46~1500

enum class IOCP_OperationType :int
{
	TYPE_ACP,					//accept�¼�������µ���������
	TYPE_RECV,				   //���ݽ����¼�
	TYPE_SEND,               //���ݷ����¼�
	TYPE_CLOSE,             //�ر��¼�
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
	SOCKET socket;		//�׽���

	//���ջ�����������AcceptEx, WSARecv����
	WSABUF recvBuf;	
	char crecvBuf[MAXBUF];

	//���ͻ������� ����WSASend����
	WSABUF sendBuf;  
	char csendBuf[MAXBUF];

	//�Զ˵�ַ
	sockaddr_in peerAddress;
};