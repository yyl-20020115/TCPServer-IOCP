#pragma once
#ifndef IOCPWRAPPER_H
#define IOCPWRAPPER_H
#include "stdafx.h"

class CIOCP
{
public:
	//CIOCP���캯��
	CIOCP(int nMaxConcurrent = -1)
		:m_hIOCP(0)
	{
		if (nMaxConcurrent != -1)
		{
			Create(nMaxConcurrent);
		}
	}
	//CIOCP��������
	~CIOCP()
	{
		if (m_hIOCP != 0)
		{
			CloseHandle(m_hIOCP);
			m_hIOCP = 0;
		}
	}

	//�ر�IOCP
	bool Close()
	{
		bool bResult = false;
		if (m_hIOCP != 0) {
			bResult = CloseHandle(m_hIOCP);
			m_hIOCP = 0;
		}
		return bResult;
	}

	//����IOCP�� nMaxConcurrencyָ������̲߳��������� 0 Ĭ��ΪCPU����
	bool Create(int nMaxConcurrency = 0)
	{
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, nMaxConcurrency);  
		return (m_hIOCP != 0);
	}

	//Ϊ�豸���ļ���socket,�ʼ��ۣ��ܵ��ȣ�����һ��IOCP
	bool AssociateDevice(HANDLE hDevice, ULONG_PTR Compkey)
	{
		bool fOk = (CreateIoCompletionPort(hDevice, m_hIOCP, Compkey, 0) == m_hIOCP);
		return fOk;
	}

	//ΪSocket����һ��IOCP
	bool AssociateSocket(SOCKET hSocket, ULONG_PTR CompKey)
	{
		return AssociateDevice((HANDLE)hSocket, CompKey);
	}

	//ΪIOCP�����¼�֪ͨ
	bool PostStatus(ULONG_PTR CompKey, DWORD dwNumBytes = 0, OVERLAPPED* po = NULL)
	{
		bool fOk = PostQueuedCompletionStatus(m_hIOCP, dwNumBytes, CompKey, po);
		return fOk;
	}
	//��IO��ɶ˿ڶ����л�ȡ�¼�֪ͨ�� IO��ɶ�����û���¼�ʱ���ú�������
	bool GetStatus(ULONG_PTR* pCompKey, PDWORD pdwNumBytes, OVERLAPPED** ppo, DWORD dwMillseconds = INFINITE)
	{
		return GetQueuedCompletionStatus(m_hIOCP, pdwNumBytes, pCompKey, ppo, dwMillseconds);
	}
	//��ȡIOCP����
	const HANDLE GetIOCPHandle()
	{
		return m_hIOCP;
	}

private:
	HANDLE m_hIOCP;   //IOCP���
};
#endif // !IOCPWRAPPER_H

