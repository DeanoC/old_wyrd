//
// Created by Computer on 16/11/2018.
//
#include "core/core.h"
#include "net/context.h"
#include "net/tcpconnection.h"
#include "net/tcpsimpleserver.h"
#include "asio.hpp"
#include <functional>
#include "asio/coroutine.hpp"

namespace Net {

struct TcpSimpleServerImpl : public asio::coroutine
{
	std::shared_ptr<TcpConnection> connection;
	std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
	TcpSimpleServer::ConnectionFunc func;
	bool childWorking = true;
	int counter = 0;

	void operator()( asio::error_code ec_ = asio::error_code(), std::size_t length_ = 0);
};

#include "asio/yield.hpp"
void TcpSimpleServerImpl::operator()(asio::error_code ec_, std::size_t length_)
{
	if (!ec_)
	{
		reenter(this)
		{
			// parent portion
			do
			{
				connection = std::make_shared<TcpConnection>();
				yield acceptor->async_accept(*(asio::ip::tcp::socket*)connection->getSocket(), *this);

				fork TcpSimpleServerImpl(*this)(ec_, length_);

			} while(is_parent());
			// parent portion
			// child only portion
			while(childWorking)
			{
				yield childWorking = !func(counter++, connection);
			}
		}
	}
}
// Disable the pseudo-keywords reenter, yield and fork.
#include "asio/unyield.hpp"

TcpSimpleServer::TcpSimpleServer(
		uint16_t port_,
		ConnectionFunc connectionFunc_)
{
	using namespace asio;
	using namespace asio::ip;

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
	GetIoContext()->stop();

	impl->acceptor->cancel();
	impl->acceptor->close();
	impl->acceptor.reset();
	impl->connection.reset();
}


}