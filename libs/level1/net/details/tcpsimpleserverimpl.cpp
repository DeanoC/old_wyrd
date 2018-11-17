#include "core/core.h"
#include "net/tcpsimpleserver.h"
#include "net/details/tcpconnectionimpl.h"
#include "net/details/tcpsimpleserverimpl.h"
#include "net/basicpayload.h"

namespace Net::Details {
TcpSimpleServerImpl::TcpSimpleServerImpl(uint16_t port_, TcpSimpleServer::ConnectionFunc connectionFunc_)
{
	using namespace asio;
	using namespace asio::ip;
	acceptor = std::make_shared<tcp::acceptor>(*GetIoContext(), tcp::endpoint(tcp::v4(), port_));
	func = connectionFunc_;
}

#include "asio/yield.hpp"
void TcpSimpleServerImpl::operator()(asio::error_code ec_, std::size_t length_)
{
	if(!ec_)
	{
		reenter(this)
		{
			// parent portion
			do
			{
				socket.reset(new asio::ip::tcp::socket(acceptor->get_executor().context()));
				yield acceptor->async_accept(*socket, *this);

				fork TcpSimpleServerImpl(*this)();

			} while(is_parent());
			// parent portion
			// child only portion
			buffer.reset(new std::array<uint8_t,8 *1024>());
			bigBuffer.reset(new std::vector<uint8_t>());

			do
			{
				yield socket->async_read_some(asio::buffer(*buffer), *this);

				// first 4 bytes are size so tell us how much we need to read
				// doesn't include the 8 bytes of the header itself
				// also include a simple end of buffer check not really serious
				// but hopefully pick up a few bugs...
				bigBuffer->resize(*((uint32_t*) buffer->data()) + 8 + 4);
				std::memset(bigBuffer->data(), 0xDC, bigBuffer->size());
				std::memcpy(bigBuffer->data(), buffer->data(), length_);
				bigBufferHead = (uint32_t)length_;

				while(bigBufferHead < (bigBuffer->size()-4))
				{
					yield socket->async_read_some(asio::buffer(*buffer), *this);
					length_ = std::min(bigBuffer->size() - bigBufferHead, length_);
					std::memcpy(bigBuffer->data() + bigBufferHead, buffer->data(), length_);
					bigBufferHead += (uint32_t)length_;
				}
				assert(*(uint32_t*)(bigBuffer->data() + bigBufferHead) == 0xDCDCDCDC);
			} while( func(*(BasicPayload*) bigBuffer->data()));

			socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec_);
			socket.reset();
		}
	}
}
// Disable the pseudo-keywords reenter, yield and fork.
#include "asio/unyield.hpp"

}