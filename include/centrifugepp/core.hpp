/*
 *	Copyright (c) 2025 Denis Rozhkov <denis@rozhkoff.com>
 *	This file is part of as-centrifugepp.
 *
 *	as-centrifugepp is free software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or (at your
 *	option) any later version.
 *
 *	as-centrifugepp is distributed in the hope that it will be
 *	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *	Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along with
 *	as-centrifugepp. If not, see <https://www.gnu.org/licenses/>.
 */

// SPDX-FileCopyrightText: 2025 Denis Rozhkov <denis@rozhkoff.com>
// SPDX-License-Identifier: GPL-3.0-or-later

/// core.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CENTRIFUGEPP__CORE__H
#define __CENTRIFUGEPP__CORE__H


#include <string>
#include <string_view>
#include <cmath>
#include <vector>
#include <atomic>


namespace as {

#define AS_T( a_t ) a_t
#define AS_STOI std::stoi
#define AS_STOLL std::stoll
#define AS_TOSTRING std::to_string
#define AS_CALL( a, ... )                                                                                              \
	if ( a ) {                                                                                                         \
		a( __VA_ARGS__ );                                                                                              \
	}


	using t_string = std::string;
	using t_char = char;
	using t_stringview = std::string_view;
	using t_byte = unsigned char;
	using t_timespan = int64_t;


	struct t_buffer {
		t_byte * ptr;
		size_t len;
		size_t size;

	public:
		t_buffer( void * ptr, size_t len )
			: ptr( static_cast<t_byte *>( ptr ) )
			, len( len )
			, size( len )
		{
		}
	};


	/// <summary>
	///
	/// </summary>
	/// <param name="buffer"></param>
	/// <returns></returns>
	inline t_string toHex( const t_buffer & buffer, const t_char * hex )
	{
		t_string out( buffer.len * 2, 0 );

		for ( size_t i = 0; i < buffer.len; i++ ) {
			out[i << 1] = hex[( buffer.ptr[i] >> 4 ) & 0x0f];
			out[( i << 1 ) + 1] = hex[buffer.ptr[i] & 0x0f];
		}

		return out;
	}


	///
	inline t_string toHex( const t_buffer & buffer )
	{
		static t_char hex[] = AS_T( "0123456789ABCDEF" );
		return toHex( buffer, hex );
	}


	///
	inline t_string toHexLowerCase( const t_buffer & buffer )
	{
		static t_char hex[] = AS_T( "0123456789abcdef" );
		return toHex( buffer, hex );
	}

} // namespace as


#endif
