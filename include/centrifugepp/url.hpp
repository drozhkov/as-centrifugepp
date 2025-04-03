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

/// url.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CENTRIFUGEPP__URL__H
#define __CENTRIFUGEPP__URL__H


#include <string>

#include "core.hpp"


namespace as {

	class Url {
	protected:
		t_string m_uri;
		t_string m_scheme;
		t_string m_hostname;
		t_string m_path;
		uint16_t m_port;

	protected:
		Url() = default;

	public:
		Url( const t_stringview & uri )
			: m_port( 0 )
		{

			parse( *this, uri.data() );
		}


		static as::t_string encode( const t_stringview & s )
		{
			as::t_string result;

			for ( size_t i = 0; i < s.length(); ++i ) {
				auto ch = s[i];

				if ( AS_T( ':' ) == ch ) {
					result.append( 1, AS_T( '%' ) );
					result.append( toHex( { const_cast<char *>( s.data() + i ), 1 } ) );
				}
				else {
					result.append( 1, ch );
				}
			}

			return result;
		}


		static void parse( Url & out, const t_stringview & s )
		{
			out.m_uri = s;

			out.m_scheme = s.substr( 0, s.find( AS_T( ':' ) ) );

			auto tokenStart = s.find( AS_T( "//" ) );
			auto tokenEnd = s.find( AS_T( "/" ), tokenStart + 2 );

			out.m_hostname = s.substr( tokenStart + 2, tokenEnd - tokenStart - 2 );

			tokenStart = out.m_hostname.find( ':' );

			if ( t_string::npos != tokenStart ) {
				out.m_port = AS_STOI( out.m_hostname.substr( tokenStart + 1 ) );

				out.m_hostname.erase( tokenStart );
			}

			out.m_path = AS_T( '/' ) + t_string( t_string::npos == tokenEnd ? AS_T( "" ) : s.substr( tokenEnd + 1 ) );

			if ( out.m_port != 0 ) {
				return;
			}

			if ( AS_T( "https" ) == out.m_scheme || AS_T( "wss" ) == out.m_scheme ) {

				out.m_port = 443;
			}
			else if ( AS_T( "http" ) == out.m_scheme || AS_T( "ws" ) == out.m_scheme ) {

				out.m_port = 80;
			}
		}


		static Url parse( const t_stringview & s )
		{
			Url out;
			parse( out, s.data() );

			return out;
		}


		Url addPath( const t_stringview & s ) const
		{
			if ( s.empty() ) {
				return Url( *this );
			}

			if ( AS_T( '/' ) == s[0] ) {
				if ( AS_T( '/' ) == m_uri[m_uri.length() - 1] ) {
					return Url( m_uri + s.substr( 1 ).data() );
				}

				return Url( m_uri + s.data() );
			}

			if ( AS_T( '/' ) == m_uri[m_uri.length() - 1] ) {
				return Url( m_uri + s.data() );
			}

			return Url( m_uri + '/' + s.data() );
		}


		Url add( const t_stringview & s ) const
		{
			return Url( m_uri + s.data() );
		}


		constexpr const t_string & Uri() const
		{
			return m_uri;
		}


		constexpr const t_string & Scheme() const
		{
			return m_scheme;
		}


		constexpr const t_string & Hostname() const
		{
			return m_hostname;
		}


		constexpr const t_string & Path() const
		{
			return m_path;
		}


		constexpr uint16_t Port() const
		{
			return m_port;
		}
	};

} // namespace as


#endif
