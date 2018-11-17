//
// Created by Computer on 17/11/2018.
//

#ifndef WYRD_NET_TCPRESOLVER_H
#define WYRD_NET_TCPRESOLVER_H

#include "core/core.h"
#include <string_view>
namespace Net {

namespace Details { struct TcpResolverImpl; }
class TcpConnection;

class TcpResolver
{
public:
	TcpResolver();
	~TcpResolver();

	[[nodiscard]] auto connect(std::string_view const& address_, uint16_t port_) ->  std::unique_ptr<TcpConnection>;
protected:
	Details::TcpResolverImpl* impl;

};

}

#endif //WYRD_NET_TCPRESOLVER_H
