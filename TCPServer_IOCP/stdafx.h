// stdafx.h : 标准系统包含文件的包含文件，
#pragma once

#include "targetver.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <WinSock2.h>
#include <MSWSock.h>
#include <assert.h>
#include "Singleton.h"
#include "IOCPWrapper.h"
#include "OverlappedIOInfo.h"
#include "TCPServerBootstrap.h"
#pragma comment(lib, "ws2_32.lib")
