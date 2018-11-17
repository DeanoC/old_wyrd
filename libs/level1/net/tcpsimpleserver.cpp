//
// Created by Computer on 16/11/2018.
//
#include "core/core.h"
#include "net/details/context.h"
#include "net/tcpconnection.h"
#include "net/tcpsimpleserver.h"
#include "net/details/tcpsimpleserverimpl.h"
#include "net/details/context.h"
#include "asio.hpp"
#include <functional>

namespace Net {


TcpSimpleServer::TcpSimpleServer(
		uint16_t port_,
		ConnectionFunc connectionFunc_)
{
	using namespace asio;
	using namespace asio::ip;
	using namespace Details;

	impl = new TcpSimpleServerImpl();
	impl->acceptor = std::make_unique<tcp::acceptor>(*GetIoContext(), tcp::endpoint(tcp::v4(), port_));
	impl->func = connectionFunc_;
}
TcpSimpleServer::~TcpSimpleServer()
{
	delete impl;
}

auto TcpSimpleServer::start() -> void
{
	using namespace Details;
	impl->operator()(); // start without errors or data

	// Wait for signals indicating time to shut down.
	asio::signal_set signals(*GetIoContext());
	signals.add(SIGINT);
	signals.add(SIGTERM);
#if defined(SIGQUIT)
	signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
	asio::io_context* ioContext = GetIoContext().get();
	signals.async_wait(std::bind(&asio::io_context::stop, ioContext));
	GetIoContext()->run();
}

auto TcpSimpleServer::stop() -> void
{
	Details::GetIoContext()->stop();

	impl->acceptor->cancel();
	impl->acceptor->close();
	impl->acceptor.reset();
	impl->connection.reset();
}


}