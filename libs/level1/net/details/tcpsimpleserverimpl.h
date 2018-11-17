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
	TcpSimpleServerImpl(uint16_t port_, TcpSimpleServer::ConnectionFunc connectionFunc_);

	std::shared_ptr<asio::ip::tcp::socket> socket;
	std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
	TcpSimpleServer::ConnectionFunc func;
	bool moreData;
	std::shared_ptr<std::array<uint8_t, 8*1024>> buffer;
	std::shared_ptr<std::vector<uint8_t>> bigBuffer;
	uint32_t bigBufferHead;

	void operator()( asio::error_code ec_ = asio::error_code(), std::size_t length_ = 0);
};


}

#endif //WYRD_TCPSIMPLESERVERIMPL_H
