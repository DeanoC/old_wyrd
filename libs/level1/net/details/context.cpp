#include "core/core.h"
#include "net/details/context.h"
#include "asio.hpp"

namespace Net::Details
{

namespace
{
std::shared_ptr<asio::io_context> context;
}

auto GetIoContext() -> std::shared_ptr<asio::io_context>
{
	if(!context)
	{
		void* mem = malloc(sizeof(asio::io_context));
		new(mem) asio::io_context();
		context = std::shared_ptr<asio::io_context>((asio::io_context*)mem,
					[](asio::io_context* mem_)
					{
						mem_->~io_context();// manually call destructor
						free(mem_); // now free the memory
					});
	}
	return context;
}

}
