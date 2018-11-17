#pragma once
#ifndef WYRD_NET_BASICPAYLOAD_H
#define WYRD_NET_BASICPAYLOAD_H

#include "core/core.h"
namespace Net
{
enum class BasicPayloadType : uint32_t;

/// used to identify basic payloads, each should be unique to a project etc. IFF like
constexpr BasicPayloadType operator "" _basic_payload_type(char const* s, size_t count)
{
	assert(count <= 4 && count > 0);
	uint32_t res =
			(s[0] << 0) |
			((count > 1 ? s[1] : uint32_t('_')) << 8) |
			((count > 2 ? s[2] : uint32_t('_')) << 16) |
			((count > 3 ? s[3] : uint32_t('_')) << 24);
	return (BasicPayloadType) res;
}

// size doesn't include the header
struct BasicPayload
{
	uint32_t size;
	BasicPayloadType type;

	void const* getPayload() const { return (this+1);}
};

}
#endif //WYRD_NET_BASICPAYLOAD_H
