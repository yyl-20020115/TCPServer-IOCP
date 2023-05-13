#pragma once
#include "TCPServerBootstrap.h"

class ProtoTCPServerBootstrap : public TCPServerBootstrap
{

public:
	ProtoTCPServerBootstrap();
	~ProtoTCPServerBootstrap();

protected:
	virtual bool DoRecv(COverlappedIOInfo* info) override;
	virtual bool DoAfterSend(COverlappedIOInfo* info) override;
};

typedef Singleton<ProtoTCPServerBootstrap> ProtoTCPServer;
