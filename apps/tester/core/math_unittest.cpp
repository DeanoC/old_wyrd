#include "tester/catch.hpp"

#include "core/core.h"
#include "core/scalar_math.h"

TEST_CASE( "pi", "[math]" )
{
	float const fpi = static_cast<float>(3.14159265358979323846264338327950L);
	double const dpi = static_cast<double>(3.14159265358979323846264338327950L);
	REQUIRE( Math::pi<float>() == Approx( fpi ));
	REQUIRE( Math::pi<double>() == Approx( dpi ));
	REQUIRE( Math::pi_over_2<float>() == Approx( fpi / 2.0f ));
	REQUIRE( Math::pi_over_2<double>() == Approx( dpi / 2.0f ));
}

TEST_CASE( "degrees2Radians", "[math]" )
{
	REQUIRE( Math::degreesToRadians<float>( 0 ) == Approx( 0.0 ));
	REQUIRE( Math::degreesToRadians<float>( 90 ) == Approx( Math::pi_over_2<float>()));
	REQUIRE( Math::degreesToRadians<float>( 180 ) == Approx( Math::pi<float>()));
	REQUIRE( Math::degreesToRadians<float>( 270 ) == Approx( Math::pi_over_2<float>() + Math::pi<float>()));
	REQUIRE( Math::degreesToRadians<float>( 360 ) == Approx( Math::pi<float>() * 2 ));

	REQUIRE( Math::degreesToRadians<double>( 0 ) == Approx( 0.0 ));
	REQUIRE( Math::degreesToRadians<double>( 90 ) == Approx( Math::pi_over_2<double>()));
	REQUIRE( Math::degreesToRadians<double>( 180 ) == Approx( Math::pi<double>()));
	REQUIRE( Math::degreesToRadians<double>( 270 ) == Approx( Math::pi_over_2<double>() + Math::pi<double>()));
	REQUIRE( Math::degreesToRadians<double>( 360 ) == Approx( Math::pi<double>() * 2 ));

}

TEST_CASE( "radians2Degrees", "[math]" )
{
	float const fpi = Math::pi<float>();
	REQUIRE( Math::radiansToDegrees<float>( 0.0f ) == Approx( 0.0f ));
	REQUIRE( Math::radiansToDegrees<float>( fpi / 2.0f ) == Approx( 90 ));
	REQUIRE( Math::radiansToDegrees<float>( fpi ) == Approx( 180 ));
	REQUIRE( Math::radiansToDegrees<float>( fpi + (fpi / 2.0f)) == Approx( 270 ));
	REQUIRE( Math::radiansToDegrees<float>( fpi * 2.0f ) == Approx( 360 ));

	double const dpi = Math::pi<double>();
	REQUIRE( Math::radiansToDegrees<double>( 0.0 ) == Approx( 0.0 ));
	REQUIRE( Math::radiansToDegrees<double>( fpi / 2.0 ) == Approx( 90 ));
	REQUIRE( Math::radiansToDegrees<double>( fpi ) == Approx( 180 ));
	REQUIRE( Math::radiansToDegrees<double>( fpi + (fpi / 2.0)) == Approx( 270 ));
	REQUIRE( Math::radiansToDegrees<double>( fpi * 2.0 ) == Approx( 360 ));

}

TEST_CASE( "square", "[math]" )
{
	REQUIRE( Math::square( 1 ) == 1 );
	REQUIRE( Math::square( -1 ) == 1 );
	REQUIRE( Math::square( 2 ) == 4 );
	REQUIRE( Math::square( -2 ) == 4 );

	REQUIRE( Math::square<uint8_t>( 1 ) == 1 );
	REQUIRE( Math::square<uint8_t>( 2 ) == 4 );
	REQUIRE( Math::square<uint8_t>( 8 ) == 64 );
	REQUIRE( Math::square<uint8_t>( 16 ) == 0 );


	REQUIRE( Math::square<float>( 1.0f ) == Approx( 1.0f ));
	REQUIRE( Math::square<float>( -1.0f ) == Approx( 1.0f ));
	REQUIRE( Math::square<float>( 2.0f ) == Approx( 4.0f ));
	REQUIRE( Math::square<float>( -2.0f ) == Approx( 4.0f ));

}