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

	std::shared_ptr<std::array<uint8_t, 8 * 1024>> receiveBuffer;
	std::shared_ptr<std::vector<uint8_t>> asmBuffer;

	uint32_t receiveHead;
	uint32_t asmHead;
	uint32_t packetSize;
	uint32_t len;

	void operator()( asio::error_code ec_ = asio::error_code(), std::size_t length_ = 0);
};


}

#endif //WYRD_TCPSIMPLESERVERIMPL_H
