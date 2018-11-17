#pragma once
#ifndef WYRD_TCPCONNECTIONIMPL_H
#define WYRD_TCPCONNECTIONIMPL_H

#include "core/core.h"
#include "net/details/context.h"
#include "asio.hpp"

namespace Net::Details
{
struct TcpConnectionImpl
{

	TcpConnectionImpl() :
		socket(asio::ip::tcp::socket(*GetIoContext().get()))
	{
	}
	asio::ip::tcp::socket socket;
};

}
#endif //WYRD_TCPCONNECTIONIMPL_H
