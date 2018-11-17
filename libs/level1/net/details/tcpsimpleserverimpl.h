#pragma once
#ifndef WYRD_NET_DETAILS_TCPSIMPLESERVERIMPL_H
#define WYRD_NET_DETAILS_TCPSIMPLESERVERIMPL_H

#include "core/core.h"
#include "net/tcpconnection.h"
#include "net/tcpsimpleserver.h"
#include "asio/coroutine.hpp"
#include "asio.hpp"

namespace Net::Details {

struct TcpSimpleServerImpl : public asio::coroutine
{
	std::shared_ptr<TcpConnection> connection;
	std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
	TcpSimpleServer::ConnectionFunc func;
	bool childWorking = true;
	int counter = 0;

	void operator()( asio::error_code ec_ = asio::error_code(), std::size_t length_ = 0);
};


}

#endif //WYRD_TCPSIMPLESERVERIMPL_H
