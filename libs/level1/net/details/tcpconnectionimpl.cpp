#include "core/core.h"
#include "net/details/tcpconnectionimpl.h"
#include "net/basicpayload.h"


namespace Net::Details {

TcpConnectionImpl::TcpConnectionImpl(std::shared_ptr<asio::io_context> const& context_)
{
	using namespace asio::ip;
	socket = std::shared_ptr<tcp::socket>(
			new tcp::socket(*context_.get()),
			[](tcp::socket* ptr_)
			{
				asio::error_code ec;
				// Initiate graceful connection closure.
				ptr_->shutdown(tcp::socket::shutdown_both, ec);
			});
}

TcpConnectionImpl::TcpConnectionImpl(std::shared_ptr<asio::ip::tcp::socket> const& socket_):
		socket(socket_)
{
}

TcpConnectionImpl::~TcpConnectionImpl()
{
	socket.reset();
}

auto TcpConnectionImpl::syncWrite(void const* data_, size_t const size_) -> void
{
	asio::write(*socket, asio::buffer(&size_, 4));
	asio::write(*socket, asio::buffer(data_, size_));
}

auto TcpConnectionImpl::syncWrite(std::string const& string_) -> void
{
	size_t const size = string_.size();
	asio::write(*socket, asio::buffer(&size, 4));
	asio::write(*socket, asio::buffer(string_.data(), size));
}

auto TcpConnectionImpl::readSize() -> size_t
{
	size_t serverSize;
	asio::read(*socket, asio::buffer(&serverSize, 4));
	return serverSize;
}

auto TcpConnectionImpl::readPayload(void* data_, size_t size_) -> size_t
{
	size_t clientSize;
	clientSize = asio::read(*socket, asio::buffer(data_, size_));
	if(clientSize != size_)
	{
		LOG_S(WARNING) << "Net: size differs between client and server, rejecting";
		return 0;
	}
	return clientSize;
}

auto TcpConnectionImpl::syncRead(void* buffer_, size_t const maxSize_) -> size_t
{
	size_t serverSize = readSize();

	if(serverSize >= maxSize_)
	{
		LOG_S(WARNING) << "Net: received buffer that is too big, rejecting";
		return 0;
	}

	return readPayload(buffer_, serverSize);
}

auto TcpConnectionImpl::syncRead(std::string& string_) -> void
{
	size_t size = readSize();
	string_.resize(size);
	readPayload(string_.data(), size);
}

auto TcpConnectionImpl::syncWriteBasicPayload(uint32_t payloadSize_, BasicPayloadType const type_,
											  void const* data_) -> void
{
	asio::write(*socket, asio::buffer(&payloadSize_, 4));
	asio::write(*socket, asio::buffer(&type_, 4));
	if(payloadSize_ != 0 && data_ != nullptr)
	{
		asio::write(*socket, asio::buffer(data_, payloadSize_));
	}
}

}