#pragma once
#include "stdafx.h"

class TCPServerBootstrap 
{
public:
	const int DefaultAcceptCount = 10;
public:
	//���캯��
	TCPServerBootstrap();
	//�鹹����
	~TCPServerBootstrap();
public:
	//��ʼ����
	bool StartListening(unsigned short port, const std::string& ip = "0.0.0.0");
	void GetConnectionClient();
	/*
	�ͷ�3�����ֲ��裺
	1. ���IOCP�̶߳��У��˳��߳�
	2. ��յȴ�accept�׽���m_vecAcps
	3. ����������׽���m_vecContInfo����ջ���
	*/
	void CloseServer();
protected:
	//����recv����
	virtual bool DoRecv(COverlappedIOInfo* info);
	//����recv����
	virtual bool DoAfterSend(COverlappedIOInfo* info);
	//˽�г�Ա����
private:
	//����CPU*2���̣߳��������������̸߳���
	size_t StartWorkThreadPool();
	//��ȡAcceptEx_�� GetAcceptExSockaddrs ����ָ��
	bool GetLPFNAcceptExAndGetAcceptSockaddrs();
	//����AcceptEx_����accept����
	bool PostAccept(COverlappedIOInfo* info);
	//����accept���� NumberOfBytes=0��ʾû���յ���һ֡���ݣ� >0ʱ��ʾ�յ��˵�һ֡����
	bool DoAccept(COverlappedIOInfo* info, DWORD NumberOfBytes = 0);
	//Ͷ��recv����
	bool PostRecv(COverlappedIOInfo* info);
	//�������ӵ�socket�б����Ƴ�socket���ͷſռ�
	bool DeleteLink(SOCKET s);

	//˽�г�Ա����
private:
	//winsock�汾����
	WSAData wsaData;
	//�˿ڼ����׽���
	SOCKET listeningSocket;
	//�ȴ�accept���׽��֣���Щ�׽��ֶ���û��ʹ�ù��ģ�����ΪACCEPT_SOCKET_NUM��ͬʱ����10���׽��ֵȴ�accept
	std::vector<SOCKET> accepts;
	//�ѽ������ӵ���Ϣ��ÿ���ṹ����һ���׽��֡����ͻ����������ջ��������Զ˵�ַ
	std::vector<COverlappedIOInfo*> conInfos;
	//����vector�Ļ��������
	std::mutex _mutex;
	//CIOCP��װ��
	CIOCP iocp;
	//AcceptEx_����ָ��
	LPFN_ACCEPTEX acceptExFunction;
	//GetAcceptSockAddrs����ָ��
	LPFN_GETACCEPTEXSOCKADDRS getAcceptSocketFunction;

	//�����̳߳�
	std::vector<std::thread> pool;
	//�����̳߳ش�С
	size_t poolSize;
};

typedef Singleton<TCPServerBootstrap> BasicTCPServer;
