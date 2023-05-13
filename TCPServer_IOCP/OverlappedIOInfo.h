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

enum class IOOperType :int
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
		: m_socket(INVALID_SOCKET)
		, m_recvBuf()
		, m_crecvBuf()
		, m_sendBuf()
		, m_csendBuf()
		, m_addr()
	{
		ResetOverlapped();
		ResetRecvBuffer();
		ResetSendBuffer();
	}
	~COverlappedIOInfo(void)
	{
		if (m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
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
		ZeroMemory(m_crecvBuf, MAXBUF);
		m_recvBuf.buf = m_crecvBuf;
		m_recvBuf.len = MAXBUF;
	}
	void ResetSendBuffer()
	{
		ZeroMemory(m_csendBuf, MAXBUF);
		m_sendBuf.buf = m_csendBuf;
		m_sendBuf.len = MAXBUF;
	}


public:
	SOCKET m_socket;		//�׽���

	//���ջ�����������AcceptEx, WSARecv����
	WSABUF m_recvBuf;	
	char m_crecvBuf[MAXBUF];

	//���ͻ������� ����WSASend����
	WSABUF m_sendBuf;  
	char m_csendBuf[MAXBUF];

	//�Զ˵�ַ
	sockaddr_in m_addr;
};