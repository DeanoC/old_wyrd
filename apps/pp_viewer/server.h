#pragma once
#ifndef WYRD_PPVIEWER_SERVER_H
#define WYRD_PPVIEWER_SERVER_H

#include "core/core.h"
#include <thread>

namespace Net { class TcpSimpleServer; }
namespace asio { class io_context; }

class Server
{
public:
	Server();
	~Server();

protected:
	std::thread serverThread;
	std::shared_ptr<Net::TcpSimpleServer> server;
};


#endif //WYRD_PPVIEWER_SERVER_H
