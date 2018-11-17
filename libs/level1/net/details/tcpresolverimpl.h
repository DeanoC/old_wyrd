#pragma once
#ifndef WYRD_NET_DETAILS_TCPRESOLVERIMPL_H
#define WYRD_NET_DETAILS_TCPRESOLVERIMPL_H

#include "core/core.h"
#include "net/details/context.h"
#include "net/details/tcpconnectionimpl.h"
#include "asio.hpp"

namespace Net::Details
{

struct TcpResolverImpl
{
	TcpResolverImpl() : resolver(*GetIoContext().get()) {}
	asio::ip::tcp::resolver resolver;

	[[no_discard]] auto connect(TcpConnectionImpl* connection_, std::string_view const& address_, std::string_view const& port_) -> bool
	{
		asio::error_code ec;
		asio::connect(connection_->socket, resolver.resolve(address_, port_), ec);
		if(ec)
		{
			LOG_S(WARNING) << "Connection error: " << ec.message();
			return false;
		}
		else
		{
			return true;
		}
	}

};

}

#endif //WYRD_NET_DETAILS_TCPRESOLVERIMPL_H
