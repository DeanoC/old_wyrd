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
	TcpResolverImpl(std::shared_ptr<asio::io_context> const& context_) : 
		weakContext(context_),
		resolver(*context_.get()) {}

	[[nodiscard]] auto connect(asio::ip::tcp::socket& socket_, std::string_view const& address_, std::string_view const& port_) -> bool
	{
		asio::error_code ec;
		asio::connect(socket_, resolver.resolve(address_, port_), ec);
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

	std::weak_ptr<asio::io_context> weakContext;
	asio::ip::tcp::resolver resolver;

};

}

#endif //WYRD_NET_DETAILS_TCPRESOLVERIMPL_H
