#pragma once
#ifndef WYRD_NET_DETAILS_CONTEXT_H
#define WYRD_NET_DETAILS_CONTEXT_H

#include "core/core.h"
namespace asio { class io_context; }

namespace Net::Details
{

// just lets us hide asio from outside libraries
auto GetIoContext() -> std::shared_ptr<asio::io_context>;

auto StopIoContext() -> void;
}

#endif //WYRD_NET_DETAILS_CONTEXT_H
