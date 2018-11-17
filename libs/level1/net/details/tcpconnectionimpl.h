#pragma once
#ifndef WYRD_TCPCONNECTIONIMPL_H
#define WYRD_TCPCONNECTIONIMPL_H

#include "core/core.h"
#include "net/tcpconnection.h"
#include "net/details/context.h"
#include "asio.hpp"

namespace Net::Details {
struct TcpConnectionImpl : public TcpConnection
{

	TcpConnectionImpl(std::shared_ptr<asio::io_context> const& context_);
	TcpConnectionImpl(std::shared_ptr<asio::ip::tcp::socket> const& socket_);
	~TcpConnectionImpl();

	auto syncWrite(void const* data_, size_t const size_) -> void final;
	auto syncWrite(std::string const& string_) -> void final;

	auto syncRead(void* buffer_, size_t const maxSize_) -> size_t final;
	auto syncRead(std::string& string_) -> void final;

	auto syncWriteBasicPayload(uint32_t payloadSize, BasicPayloadType const type_, void const* data_) -> void final;

	auto readSize() -> size_t;
	auto readPayload(void* data_, size_t size_) -> size_t;

	std::shared_ptr<asio::ip::tcp::socket> socket;
};

}
#endif //WYRD_TCPCONNECTIONIMPL_H
