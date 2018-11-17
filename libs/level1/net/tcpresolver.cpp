#include "core/core.h"
#include "net/tcpresolver.h"
#include "net/details/tcpresolverimpl.h"
#include "net/tcpconnection.h"
#include "net/details/tcpconnectionimpl.h"

namespace Net
{

TcpResolver::TcpResolver()
{
	impl = new Details::TcpResolverImpl(Details::GetIoContext());
}

TcpResolver::~TcpResolver()
{
	delete impl;
}

auto TcpResolver::connect(std::string_view const& address_, uint16_t port_) -> std::unique_ptr<TcpConnection>
{
	using namespace Details;
	LOG_S(INFO) << "Resolving TCP address: " << address_ << ":" << port_;

	auto connection = std::make_unique<TcpConnectionImpl>(impl->weakContext.lock());
	if(impl->connect(*connection->socket.get(), address_, std::to_string(port_)))
	{
		return std::move(connection);
	} else return {};
}


}