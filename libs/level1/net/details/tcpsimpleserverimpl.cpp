#include "core/core.h"
#include "net/tcpsimpleserver.h"
#include "net/details/tcpconnectionimpl.h"
#include "net/details/tcpsimpleserverimpl.h"

namespace Net::Details {

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
				connection = std::make_shared<TcpConnection>();
				yield acceptor->async_accept(connection->getImpl()->socket, *this);

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

}