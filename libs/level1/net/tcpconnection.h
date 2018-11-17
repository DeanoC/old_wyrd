#pragma once
#ifndef WYRD_NET_TCPCONNECTION_H
#define WYRD_NET_TCPCONNECTION_H

#include "core/core.h"
#include <vector>
#include <array>

namespace Net
{
struct BasicPayload;
enum class BasicPayloadType : uint32_t;

class TcpConnection
{
public:
	virtual ~TcpConnection(){};

	virtual auto syncWrite( void const* data_, size_t const size_) -> void = 0;
	virtual auto syncWrite( std::string const& string_) -> void = 0;

	virtual auto syncRead( void* buffer_, size_t const maxSize_ ) -> size_t = 0;
	virtual auto syncRead( std::string& string_) -> void = 0;

	virtual auto syncWriteBasicPayload(uint32_t payloadSize, BasicPayloadType const type_, void const* data_) -> void = 0;

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
};

}

#endif //WYRD_NET_TCPCONNECTION_H
