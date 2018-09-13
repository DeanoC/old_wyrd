#pragma once
#if !defined(PLAYFIELD_HEXFIELD_H_)
#define PLAYFIELD_HEXFIELD_H_

#include "core/core.h"
#include "core/vector_math.h"
#include <functional>
#include "core/utils.h"
#include "cx/cx_array.h"

namespace Binny { class Bundle; class WriteHelper; };

namespace Playfield {

	enum class PointyHexDirections
	{
		E = 0,
		SE,
		SW,
		W,
		NW,
		NE,
	};

	// http://www.gamelogic.co.za/downloads/HexMath2.pdf
	template<typename CoordType>
	class BaseHexCoord {
	public:
		static_assert(std::is_signed<CoordType>(), "HexCoord must be signed");

		constexpr BaseHexCoord() {}
		constexpr explicit BaseHexCoord(CoordType x_, CoordType y_) : x(x_), y(y_){}
		constexpr BaseHexCoord(PointyHexDirections dir_) : x(GetPointyHexDirection<CoordType>(dir_).x), y(GetPointyHexDirection<CoordType>(dir_).y) {}

		constexpr CoordType z() const { return - x - y; }

		constexpr explicit BaseHexCoord(Math::Vector2 const& rhs) {
			float const fix = std::round(rhs.x);
			float const fiy = std::round(rhs.y);
			assert(fix <= std::numeric_limits<CoordType>::max() && fix >= std::numeric_limits<CoordType>::min());
			assert(fiy <= std::numeric_limits<CoordType>::max() && fiy >= std::numeric_limits<CoordType>::min());

			x = (CoordType)fix;
			y = (CoordType)fiy;
		}
		constexpr friend BaseHexCoord operator+(BaseHexCoord const& lhs, BaseHexCoord const& rhs)
		{
			return BaseHexCoord(lhs.x + rhs.x, lhs.y + rhs.y);
		}
		constexpr friend BaseHexCoord operator-(BaseHexCoord const& lhs, BaseHexCoord const& rhs)
		{
			return BaseHexCoord(lhs.x - rhs.x, lhs.y - rhs.y);
		}

		static constexpr float euclideanNorm(BaseHexCoord const& a) {
			return std::sqrt(Math::square(float(a.x)) + Math::square(float(a.y)) + Math::square(float(a.z())) / 2.0f);
		}
		static constexpr float euclideanDistance(BaseHexCoord const& lhs, BaseHexCoord const& rhs) {
			return euclideanNorm(lhs - rhs);
		}
		static constexpr float hexNorm(BaseHexCoord const& a) {
			return Math::HorizMax(Math::Abs(Math::Vector3(a.x, a.y, a.z())));
		}
		static constexpr float hexDistance(BaseHexCoord const& lhs, BaseHexCoord const& rhs) {
			return hexNorm(lhs - rhs);
		}
		static constexpr float downTrianglePseudoNorm(BaseHexCoord const& a) {
			return Math::HorizMax(Math::Vector3(a.x, a.y, a.z()));
		}
		static constexpr float upTrianglePseudoNorm(BaseHexCoord const& a) {
			return Math::HorizMax(-Math::Vector3(a.x, a.y, a.z()));
		}
		static constexpr float starNorm(BaseHexCoord const& a)
		{
			return Math::Min(downTrianglePseudoNorm(a), upTrianglePseudoNorm(b));
		}

		static constexpr BaseHexCoord round(Math::Vector2 const& coord) {
			float const fix = std::round(coord.x);
			float const fiy = std::round(coord.y);
			assert(fix <= std::numeric_limits<CoordType>::max() && fix >= std::numeric_limits<CoordType>::min());
			assert(fiy <= std::numeric_limits<CoordType>::max() && fiy >= std::numeric_limits<CoordType>::min());

			float const z = -coord.x - coord.y;
			float const fiz = std::round(z);

			int const axis = Math::MajorAxis(Math::Vector3(coord.x - fix, coord.y - fiy, z - fiz));
			assert( ((axis == 0) && (-fiy - fiz) <= std::numeric_limits<CoordType>::max() && (-fiy - fiz) >= std::numeric_limits<CoordType>::min()) ||
					((axis == 1) && (-fix - fiz) <= std::numeric_limits<CoordType>::max() && (-fix - fiz) >= std::numeric_limits<CoordType>::min()) ||
					(axis == 2));
			switch (axis)
			{
			case 0: return BaseHexCoord(CoordType(-fiy - fiz), CoordType(fiy));
			case 1: return BaseHexCoord(CoordType(fix), CoordType(-fix - fiz));
			case 2: return BaseHexCoord(CoordType(fix), CoordType(fiy));
			default: return {};
			}
		}

		static constexpr Math::Vector2 pointyHexCorner(BaseHexCoord<CoordType> coord_, int index_)
		{
			const float angle = Math::degreesToRadians((60.0f * index_) - 30.0f);
			return { float(coord_) + cx::cos(angle), float(coord_) + cx::sin(angle) };
		}

		CoordType x, y;


	};
	 template<typename CoordType> constexpr BaseHexCoord<CoordType> GetPointyHexDirection(PointyHexDirections dir_) {
		constexpr auto d = cx::make_array<BaseHexCoord<CoordType>>(
			BaseHexCoord(CoordType(+1), CoordType(+0)),
			BaseHexCoord(CoordType(+0), CoordType(+1)),
			BaseHexCoord(CoordType(-1), CoordType(+1)),
			BaseHexCoord(CoordType(-1), CoordType(+0)),
			BaseHexCoord(CoordType(+0), CoordType(-1)),
			BaseHexCoord(CoordType(+1), CoordType(-1))
		);
		return d[(int)dir_];
	};

	struct HexTile {

	};

	class HexMap
	{
	};
}
#endif