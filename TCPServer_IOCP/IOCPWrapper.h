#pragma once
#ifndef IOCPWRAPPER_H
#define IOCPWRAPPER_H
#include "stdafx.h"

class CIOCP
{
public:
	//CIOCP���캯��
	CIOCP(int nMaxConcurrent = -1)
		:handle(0)
	{
		if (nMaxConcurrent != -1)
		{
			Create(nMaxConcurrent);
		}
	}
	//CIOCP��������
	~CIOCP()
	{
		if (handle != 0)
		{
			CloseHandle(handle);
			handle = 0;
		}
	}

	//�ر�IOCP
	bool Close()
	{
		bool result = false;
		if (handle != 0) {
			result = CloseHandle(handle);
			handle = 0;
		}
		return result;
	}

	//����IOCP�� nMaxConcurrencyָ������̲߳��������� 0 Ĭ��ΪCPU����
	bool Create(int nMaxConcurrency = 0)
	{
		handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, nMaxConcurrency);  
		return (handle != 0);
	}

	//Ϊ�豸���ļ���socket,�ʼ��ۣ��ܵ��ȣ�����һ��IOCP
	bool AssociateDevice(HANDLE hDevice, ULONG_PTR Compkey)
	{
		bool fOk = (CreateIoCompletionPort(hDevice, handle, Compkey, 0) == handle);
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
		bool fOk = PostQueuedCompletionStatus(handle, dwNumBytes, CompKey, po);
		return fOk;
	}
	//��IO��ɶ˿ڶ����л�ȡ�¼�֪ͨ�� IO��ɶ�����û���¼�ʱ���ú�������
	bool GetStatus(ULONG_PTR* pCompKey, PDWORD pdwNumBytes, OVERLAPPED** ppo, DWORD dwMillseconds = INFINITE)
	{
		return GetQueuedCompletionStatus(handle, pdwNumBytes, pCompKey, ppo, dwMillseconds);
	}
	//��ȡIOCP����
	const HANDLE GetHandle()
	{
		return handle;
	}

private:
	HANDLE handle;   //IOCP���
};
#endif // !IOCPWRAPPER_H

