#include "stdafx.h"
#include "ProtoTCPServerBootstrap.h"

ProtoTCPServerBootstrap::ProtoTCPServerBootstrap()
{
}

ProtoTCPServerBootstrap::~ProtoTCPServerBootstrap()
{
}

bool ProtoTCPServerBootstrap::DoRecv(COverlappedIOInfo* info)
{
    return false;
}

bool ProtoTCPServerBootstrap::DoAfterSend(COverlappedIOInfo* info)
{
    return false;
}
