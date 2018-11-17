#pragma once
#ifndef WYRD_NET_TCPCONNECTION_H
#define WYRD_NET_TCPCONNECTION_H

#include <vector>
#include <array>

namespace Net
{
namespace Details { struct TcpConnectionImpl; }

class TcpConnection
{
public:
	TcpConnection();
	~TcpConnection();

	auto syncWrite( void const* data_, size_t const size_) -> void;
	auto syncWrite( std::string const& string_) -> void;

	auto syncRead( void* buffer_, size_t const maxSize_ ) -> size_t;
	auto syncRead( std::string& string_) -> void;

	template<typename T> auto syncWrite(std::vector<T> const& vector_) -> void
	{
		syncWrite(vector_.data(), sizeof(T) * vector_.size());
	}

	template<typename T, size_t S> auto syncWrite(std::array<T,S> const& array_) -> void
	{
		syncWrite(array_.data(), sizeof(T) * array_.size());
	}

	template<typename T, bool preAllocated_ = false>
	auto syncRead(std::vector<T>& vector_ ) -> size_t
	{
		static_assert(sizeof(T) != 0);

		if constexpr (preAllocated_)
		{
			return syncRead(vector_.data(), sizeof(T) * vector_.size());
		}
		else
		{
			size_t size = readSize();
			vector_.resize(size/sizeof(T));
			return readPayload(vector_.data(), sizeof(T) * vector_.size());
		}
	}
	template<typename T, size_t S = false>
	auto syncRead(std::array<T,S>& array_ ) -> size_t
	{
		static_assert(sizeof(T) != 0);
		static_assert(S > 0);
		return syncRead(array_.data(), sizeof(T) * array_.size());
	}

	auto getImpl() const -> Details::TcpConnectionImpl* const { return impl; };
	auto getImpl() -> Details::TcpConnectionImpl* { return impl; };

private:
	auto readSize() -> size_t;
	auto readPayload(void* data_, size_t size_) -> size_t;

	Details::TcpConnectionImpl* impl;
};

}

#endif //WYRD_NET_TCPCONNECTION_H
