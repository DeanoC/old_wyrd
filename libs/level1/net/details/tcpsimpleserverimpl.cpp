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

			receiveBuffer.reset(new std::array<uint8_t, 8 * 1024>());
			asmBuffer.reset(new std::vector<uint8_t>());

			receiveHead = 0;
			do
			{
				packetSize = 0;

				// starting a packet or still recieving parts it
				while(packetSize == 0 || asmHead < packetSize)
				{
					if(receiveHead >= length_)
					{
						yield socket->async_read_some(asio::buffer(*receiveBuffer),
																						*this);
						receiveHead = 0;
					}

					if(asmHead >= packetSize)
					{
						// 8 byte header + the payload
						packetSize = 8 + *((uint32_t*) (receiveBuffer->data() + receiveHead));
						asmBuffer->resize(packetSize);
						std::memset(asmBuffer->data(), 0xDC, asmBuffer->size());
						asmHead = 0;
					}

					len = std::min(packetSize - asmHead, (uint32_t) length_ - receiveHead);
					std::memcpy(asmBuffer->data() + asmHead, receiveBuffer->data() + receiveHead, len);
					asmHead += len;
					receiveHead += len;
				}
			} while(func(*(BasicPayload*) asmBuffer->data()));

			socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec_);
			socket.reset();
		}
	}
}
// Disable the pseudo-keywords reenter, yield and fork.
#include "asio/unyield.hpp"

}