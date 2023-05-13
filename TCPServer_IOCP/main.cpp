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
			//����1  �ر�TCP������
			server->CloseServer();
			break;
		}
		else
		{
			//�����������֣���ӡ�������߿ͻ�����Ϣ
			server->GetConnectionClient();
		}
	}
	Sleep(5000);
	return 0;
}
