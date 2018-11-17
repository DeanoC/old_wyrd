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
	impl = new Details::TcpSimpleServerImpl(port_, connectionFunc_);
}
TcpSimpleServer::~TcpSimpleServer()
{
	delete impl;
}

auto TcpSimpleServer::start() -> void
{
	using namespace Details;
	(*impl)(); // start without errors or data

	// Wait for signals indicating time to shut down.
	asio::signal_set signals(*GetIoContext());
	signals.add(SIGINT);
	signals.add(SIGTERM);
#if defined(SIGQUIT)
	signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
	asio::io_context* ioContext = GetIoContext().get();
	signals.async_wait(std::bind(&asio::io_context::stop, ioContext));

	std::vector<std::shared_ptr<std::thread> > threads;
	for (std::size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
		threads.push_back(
			std::make_shared<std::thread>(std::bind(&asio::io_context::run, GetIoContext().get())));
		threads[i]->join();
	}

	GetIoContext()->run();
}

auto TcpSimpleServer::stop() -> void
{
	Details::GetIoContext()->stop();

	impl->acceptor->cancel();
	impl->acceptor->close();
	impl->acceptor.reset();
	impl->socket.reset();
//	impl->connection.reset();
}


}