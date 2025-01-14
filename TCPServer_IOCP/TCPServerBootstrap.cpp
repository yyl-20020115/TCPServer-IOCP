#include "stdafx.h"
#include "TCPServerBootstrap.h"
/*构造函数*/

TCPServerBootstrap::TCPServerBootstrap()
	: acceptExFunction(NULL)
	, getAcceptSocketFunction(NULL)
	, poolSize(0)
	, listeningSocket(INVALID_SOCKET)
	, wsaData()
	, accepts()
	, conInfos()
	, _mutex()
	, iocp()
	, pool()
{
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
}

/*析构函数*/
TCPServerBootstrap::~TCPServerBootstrap()
{
	CloseServer();
	WSACleanup();
}

/*TCP Server开启监听*/
bool TCPServerBootstrap::StartListening(unsigned short port, const std::string& ip)
{
	//listen socket需要将accept操作投递到完成端口，因此listen socket属性必须有重叠IO
	listeningSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listeningSocket == INVALID_SOCKET)
	{
		std::cerr << "WSASocket socket failed!" << std::endl;
		return false;
	}
	std::cout << "WSASocket succeeded!" << std::endl;
	//创建并设置IOCP并发线程数量
	if (!iocp.Create())
	{
		std::cerr << "IOCP error, error code :" << WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "IOCP succeeded!" << std::endl;
	//将listen socket绑定至IOCP
	if (!iocp.AssociateSocket(listeningSocket, (ULONG_PTR)IOCP_OperationType::TYPE_ACP))
	{
		std::cerr << "IOCP associate listening failed!" << std::endl;
		return false;
	}
	std::cout << "IOCP associate listening succeeded!" << std::endl;
	//创建Socket
	sockaddr_in service{};
	service.sin_family = AF_INET;
	service.sin_port = htons(port);
	if (ip.empty())
	{
		service.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		service.sin_addr.s_addr = inet_addr(ip.c_str());
	}
	if (::bind(listeningSocket, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		std::cerr << "Socket bind error! error code :" << WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "Socket binding succeeded!" << std::endl;
	if (::listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Socket listening error! error code :" << WSAGetLastError() << std::endl;
	}
	std::cout << "Socket listening succeeded!!" << std::endl;
	//启动工作者线程
	size_t count = StartWorkThreadPool();
	std::cout << "Start working thread pool, thread count :" << count << std::endl;
	//获取AcceptEx_和GetAcceptSockAddrs函数指针
	if (!GetLPFNAcceptExAndGetAcceptSockaddrs())
	{
		std::cerr << "GetLPFNAcceptExAndGetAcceptSockaddrs error!!!" << std::endl;
		return false;
	}
	//std::cout << "GetLPFNAcceptExAndGetAcceptSockaddrs Success!!!" << std::endl;
	//创建DefaultAcceptCount个AcceptEx
	for (int i = 0; i < DefaultAcceptCount; i++)
	{
		COverlappedIOInfo* info = new COverlappedIOInfo();
		if (!PostAccept(info))
		{
			delete info;
			return false;
		}
	}
	return true;
}

/*获取所有在线的客户端信息，并打印*/
void TCPServerBootstrap::GetConnectionClient()
{
	std::cout << "Connections List: " << std::endl;
	std::for_each(conInfos.begin(), conInfos.end(),
		[](COverlappedIOInfo* olinfo)
		{
			std::cout << "        Client "
				<< inet_ntoa(olinfo->peerAddress.sin_addr)
				<< ":" << olinfo->peerAddress.sin_port
				<< std::endl;
		});
}


/*启动工作者线程池*/
size_t TCPServerBootstrap::StartWorkThreadPool()
{
	if (poolSize == 0)
	{
		poolSize = std::thread::hardware_concurrency();
	}
	for (int i = 0; i < poolSize; i++)
	{
		pool.emplace_back(
			[this]()
			{
				//std::cout << "work thread id=" << std::this_thread::get_id <<std::endl;
				while (true)
				{
					DWORD NumberOfBytes = 0;
					ULONG_PTR completionKey = 0;
					OVERLAPPED* ol = NULL;
					//阻塞调用GetQueuedCompletionStatus获取完成端口事件
					//GetQueuedCompletionStatus成功返回
					if (GetQueuedCompletionStatus(this->iocp.GetHandle(), &NumberOfBytes, &completionKey, &ol, WSA_INFINITE))
					{
						COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
						if (completionKey == (ULONG_PTR)IOCP_OperationType::TYPE_CLOSE)
						{
							break;
						}
						//客户端断开连接
						if (NumberOfBytes == 0 && (completionKey == (ULONG_PTR)IOCP_OperationType::TYPE_RECV || completionKey == (ULONG_PTR)IOCP_OperationType::TYPE_SEND))
						{
							std::cout << "Client " << inet_ntoa(olinfo->peerAddress.sin_addr) << ":" << olinfo->peerAddress.sin_port << " closed!!!" << std::endl;
							this->DeleteLink(olinfo->socket);
							continue;
						}
						switch (completionKey)
						{
							//客户端接入完成事件句柄
							case (ULONG_PTR)IOCP_OperationType::TYPE_ACP:
							{
								this->DoAccept(olinfo, NumberOfBytes);
								this->PostAccept(olinfo);
							}
							break;
							//数据接收完成事件句柄
							case (ULONG_PTR)IOCP_OperationType::TYPE_RECV:
							{
								this->DoRecv(olinfo);
								this->PostRecv(olinfo);
							}
							break;
							//数据发送完成事件句柄
							case (ULONG_PTR)IOCP_OperationType::TYPE_SEND:
							{
								this->DoAfterSend(olinfo);
							}
							break;
							//默认处理句柄
							default:
								break;
						}
					}
					//GetQueuedCompletionStatus出错
					else
					{
						int res = WSAGetLastError();
						switch (res)
						{
						case ERROR_NETNAME_DELETED:
						{
							COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
							if (olinfo)
							{
								std::cerr << "Client " << inet_ntoa(olinfo->peerAddress.sin_addr) << ":" << olinfo->peerAddress.sin_port << "exception exit!!!" << std::endl;
								this->DeleteLink(olinfo->socket);
							}
						}break;
						default:
						{
							std::cerr << "working thread GetQueuedCompletionStatus error, error code :" << WSAGetLastError() << std::endl;
						}break;
						}
					}
				}
				std::cout << "Work thread id " << GetCurrentThreadId() << " stopped!" << std::endl;
			});
	}
	return poolSize;
}

/*GetLPFNAcceptExAndGetAcceptSockaddrs 获取相应函数指针*/
bool TCPServerBootstrap::GetLPFNAcceptExAndGetAcceptSockaddrs()
{
	DWORD ByteReturned = 0;
	//获取AcceptEx函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	if (SOCKET_ERROR == WSAIoctl(
		listeningSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&acceptExFunction,
		sizeof(acceptExFunction),
		&ByteReturned,
		NULL, NULL
	))
	{
		std::cerr << "WSAIoctl get AcceptEx function error, error code :" << WSAGetLastError() << std::endl;
		return false;
	}
	//获取GetAcceptExSockAddrs函数指针
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (SOCKET_ERROR == WSAIoctl(
		listeningSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&getAcceptSocketFunction,
		sizeof(getAcceptSocketFunction),
		&ByteReturned,
		NULL, NULL
	))
	{
		std::cerr << "WSAIoctl get GetAcceptExSockAddrs function error, error code :" << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

/*投递AcceptEx事件*/
bool TCPServerBootstrap::PostAccept(COverlappedIOInfo* info)
{
	if (acceptExFunction == NULL)
	{
		std::cerr << "m_lpfnAcceptEx is not found!" << std::endl;
		return false;
	}
	SOCKET socket = info->socket;
	info->ResetRecvBuffer();
	info->ResetSendBuffer();
	info->ResetOverlapped();
	info->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (info->socket == INVALID_SOCKET)
	{
		std::cerr << "WSASocket error, error code :" << WSAGetLastError() << std::endl;
		return false;
	}
	/*
	这里建立的socket用来和对端建立连接，最终会被加入m_vecContInfo列表中
	调用AcceptEx将accept socket绑定至完成端口， 并开始进行事件监听
	这里需要传递Overlapped， new一个COverlappedIOInfo
	AcceptEx是m_listen的监听事件， m_listen已经绑定了完成端口；虽然info->m_socket已经创建，但是未使用，
	现在不必为info->m_socket绑定完成端口， 在AcceptEx事件发生后，再为info->m_socket绑定IOCP
	*/
	DWORD byteReceived = 0;
	if (false == acceptExFunction
	(
		listeningSocket,
		info->socket,
		info->recvBuf.buf,
		info->recvBuf.len - (sizeof(SOCKADDR_IN) + 16) * 2,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&byteReceived,
		info
	))
	{
		DWORD res = WSAGetLastError();
		if (ERROR_IO_PENDING != res)
		{
			std::cerr << "AcceptEx error, error code :" << res << std::endl;
			return false;
		}
	}
	std::vector<SOCKET>::iterator iter = accepts.begin();
	for (; iter != accepts.end(); ++iter)
	{
		if (*iter == socket)
		{
			*iter = info->socket;
		}
	}
	//列表中没有找到socket
	if (iter == accepts.end())
	{
		accepts.push_back(info->socket);
	}
	return true;
}

/*有客户的接入，客户端接入事件处理句柄*/
bool TCPServerBootstrap::DoAccept(COverlappedIOInfo* info, DWORD NumberOfBytes)
{
	/*
	分支用于获取远端地址
	如果接收TYPE_ACP的同时，接收到了第一帧数据，则第一帧数据中包含远端地址
	如果没有收到第一帧数据，则通过getpeername获取远端地址
	*/
	SOCKADDR_IN ClientAddr{};
	int remoteLen = sizeof(SOCKADDR_IN);
	if (NumberOfBytes > 0)
	{
		/*接收的数据分为3部分：1. 客户端发来的数据  2. 本地地址  3. 远端地址*/
		if (getAcceptSocketFunction)
		{
			SOCKADDR_IN LocalAddr{};
			int localLen = sizeof(SOCKADDR_IN);
			getAcceptSocketFunction(
				info->recvBuf.buf,
				info->recvBuf.len - (sizeof(SOCKADDR_IN) + 16) * 2,
				sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16,
				(LPSOCKADDR*)&LocalAddr,
				&localLen,
				(LPSOCKADDR*)&ClientAddr,
				&remoteLen
			);
			std::cout << "new client connection from " << inet_ntoa(ClientAddr.sin_addr) << ":" << ClientAddr.sin_port
				<< " Current Thread " << GetCurrentThreadId()
				<< std::endl << "Recved Data: " << info->recvBuf.buf << std::endl;
		}
		//未收到第一帧数据
		else if (NumberOfBytes == 0)
		{
			std::cout << "New Connection!" << std::endl;
			if (SOCKET_ERROR == getpeername(info->socket, (sockaddr*)&ClientAddr, &remoteLen))
			{
				std::cerr << "getpeername error, error code :" << WSAGetLastError() << std::endl;
			}
			else
			{
				std::cout << "new client connection from " << inet_ntoa(ClientAddr.sin_addr) << ":" << ClientAddr.sin_port << std::endl;
			}
		}

		COverlappedIOInfo* olinfo = new COverlappedIOInfo();
		olinfo->socket = info->socket;
		olinfo->peerAddress = ClientAddr;
		//服务端只收取recv，同时监听recv和send可用设计位偏移， 用或运算实现
		if (iocp.AssociateSocket(olinfo->socket, (ULONG_PTR)IOCP_OperationType::TYPE_RECV))
		{
			PostRecv(olinfo);
			conInfos.push_back(olinfo);
		}
		else
		{
			delete olinfo;
			return false;
		}
	}
	return true;
}

/*投递一个接收数据完成端口*/
bool TCPServerBootstrap::PostRecv(COverlappedIOInfo* info)
{
	DWORD BytesRecevd = 0;
	DWORD dwFlags = 0;
	info->ResetOverlapped();
	info->ResetRecvBuffer();
	int recvNum = WSARecv(info->socket, &info->recvBuf, 1, &BytesRecevd, &dwFlags, (OVERLAPPED*)info, NULL);
	if (recvNum != 0)
	{
		int res = WSAGetLastError();
		if (WSA_IO_PENDING != res)
		{
			std::cerr << "WSARecv error, error code:" << res << std::endl;
		}
	}
	return true;
}

/*接收数据处理句柄*/
bool TCPServerBootstrap::DoRecv(COverlappedIOInfo* info)
{
	std::cout << "Received data from " << inet_ntoa(info->peerAddress.sin_addr) << ":" << info->peerAddress.sin_port
		<< " Current Thread id " << GetCurrentThreadId()
		<< std::endl << "Recved Data: " << info->recvBuf.buf << std::endl;
	/*回声服务测试 echo service*/
	//投递一个发送数据完成端口

	return true;
}
bool TCPServerBootstrap::DoAfterSend(COverlappedIOInfo* info) 
{
	return true;
}

/*删除失效连接句柄*/
bool TCPServerBootstrap::DeleteLink(SOCKET s)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::vector<COverlappedIOInfo*>::iterator iter = conInfos.begin();
	bool found = false;   //标志位，客户端在列表中找到与否
	for (; iter != conInfos.end(); ++iter)
	{
		if (s == (*iter)->socket)
		{
			COverlappedIOInfo* ol = *iter;
			::closesocket(s);
			conInfos.erase(iter);
			delete ol;
			found = true;   //找到了
			break;
		}
	}
	if (!found)  //客户端在列表中没有找到
	{
		std::cerr << "socket is not in m_vecContInfo" << std::endl;
		return false;
	}
	return true;
}

/*关闭服务器*/
void TCPServerBootstrap::CloseServer()
{
	//1. 清空IOCP线程队列， 退出工作线程， 给所有的线程发送PostQueueCompletionStatus信息
	for (int i = 0; i < poolSize; i++)
	{
		if (!iocp.PostStatus((ULONG_PTR)IOCP_OperationType::TYPE_CLOSE))
		{
			std::cerr << "PostQueuedCompletionStatus error, error code :" << WSAGetLastError() << std::endl;
		}
	}
	std::cout << "Cleaned IOCP work threads" << std::endl;
	//2. 清空等待accept的套接字m_vecAcps
	for (auto s : accepts)
	{
		::closesocket(s);
	}
	accepts.clear();
	//3. 清空已连接的套接字m_vecContInfo并清空缓存
	for (auto olinfo : conInfos) {
		::closesocket(olinfo->socket);
		delete olinfo;
	}
	conInfos.clear();
}
