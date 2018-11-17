#include "core/core.h"
#include "net/tcpresolver.h"
#include "net/tcpconnection.h"
#include "net/details/tcpresolverimpl.h"

namespace Net
{

TcpResolver::TcpResolver()
{
	impl = new Details::TcpResolverImpl();
}

TcpResolver::~TcpResolver()
{
	delete impl;
}

auto TcpResolver::connect(std::string_view const& address_, uint16_t port_) -> std::unique_ptr<TcpConnection>
{
	LOG_S(INFO) << "Resolving TCP address: " << address_ << ":" << port_;

	auto connection = std::make_unique<TcpConnection>();
	impl->connect(connection->getImpl(), address_, std::to_string(port_));

	return connection;
}


}