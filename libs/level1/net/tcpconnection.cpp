//
// Created by Computer on 16/11/2018.
//
#include "core/core.h"
#include "net/context.h"
#include "net/tcpconnection.h"
#include "asio.hpp"

namespace Net {

struct TcpConnectionImpl
{
	TcpConnectionImpl(asio::ip::tcp::socket&& socket_) : socket(std::move(socket_)) {};
	asio::ip::tcp::socket socket;
};

TcpConnection::TcpConnection()
{
	impl = new TcpConnectionImpl(asio::ip::tcp::socket(*GetIoContext().get()));
}
TcpConnection::~TcpConnection()
{
	using namespace asio::ip;
	asio::error_code ec;
	// Initiate graceful connection closure.
	impl->socket.shutdown(tcp::socket::shutdown_both, ec);
	delete impl;
}
auto TcpConnection::syncWrite(void const* data_, size_t const size_) -> void
{
	asio::write(impl->socket, asio::buffer(&size_, 8));
	asio::write(impl->socket, asio::buffer(data_, size_));
}

auto TcpConnection::syncWrite(std::string const& string_) -> void
{
	size_t const size = string_.size();
	asio::write(impl->socket, asio::buffer(&size, 8));
	asio::write(impl->socket, asio::buffer(string_.data(), size));
}

auto TcpConnection::readSize() -> size_t
{
	size_t serverSize;
	asio::read(impl->socket, asio::buffer(&serverSize, 8));
	return serverSize;
}

auto TcpConnection::readPayload(void* data_, size_t size_) -> size_t
{
	size_t clientSize;
	clientSize = asio::read(impl->socket, asio::buffer(data_, size_));
	if(clientSize != size_)
	{
		LOG_S(WARNING) << "Net: size differs between client and server, rejecting";
		return 0;
	}
	return clientSize;
}

auto TcpConnection::syncRead(void* buffer_, size_t const maxSize_) -> size_t
{
	size_t serverSize = readSize();

	if(serverSize >= maxSize_)
	{
		LOG_S(WARNING) << "Net: received buffer that is too big, rejecting";
		return 0;
	}

	return readPayload(buffer_, serverSize);
}

auto TcpConnection::syncRead(std::string& string_) -> void
{
	size_t size = readSize();
	string_.resize(size);
	readPayload(string_.data(), size);
}

auto TcpConnection::getSocket() const -> InternalSocket const
{
	return &impl->socket;
}

auto TcpConnection::getSocket() -> InternalSocket
{
	return &impl->socket;
}

}