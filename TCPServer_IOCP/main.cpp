#include "stdafx.h"
#include "ProtoTCPServerBootstrap.h"

int main(int argc, char *argv[])
{
	auto server = ProtoTCPServer::GetInstance();

	if (!server->StartListening(12345, "0.0.0.0")) {
		std::cout << "Failed to start listening, quit." << std::endl;
		return -1;
	}
	
	std::cout << "Input 1 to exit, other chars for printing stats..." << std::endl;

	int cmd = 0;
	
	while (std::cin >> cmd)
	{
		if (cmd == 1)
		{
			//输入1  关闭TCP服务器
			server->CloseServer();
			break;
		}
		else
		{
			//输入任意数字，打印所有在线客户端信息
			server->GetConnectionClient();
		}
	}
	Sleep(5000);
	return 0;
}
