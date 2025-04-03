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

/// centrifugeClient.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "protocol/client.pb.h"

#include "centrifugepp/centrifugeClient.hpp"


namespace as {

	void CentrifugeClientBase::wsHandshakeHandler( as::WsClientBase & client )
	{
		try {
			centrifugal::centrifuge::protocol::Command command;
			command.set_id( m_messageId.fetch_add( 1 ) );
			auto connectRequest = command.mutable_connect();
			connectRequest->set_token( Token() );

			std::string message;
			serialize( message, command );
			AS_LOG_DEBUG_LINE( "sending message..." << as::toHex( as::t_buffer( message.data(), message.size() ) ) );

			client.write( message.data(), message.size() );
			client.readAsync();
		}
		catch ( const std::exception & e ) {
			AS_LOG_ERROR_LINE( e.what() );
		}
		catch ( ... ) {
			AS_LOG_ERROR_LINE( "Unknown exception" );
		}
	}


	bool CentrifugeClientBase::wsReadHandler( as::WsClientBase & client, const char * data, size_t size )
	{
		try {
			as::t_buffer b( const_cast<char *>( data ), size );
			AS_LOG_DEBUG_LINE( "Read: " << size );

			do {
				auto s = decodeLength( b );

				if ( 0 == s ) {
					client.write( "\0", 1 );
					break;
				}

				AS_LOG_DEBUG_LINE( "Size: " << s << " Len size: " << b.len );

				centrifugal::centrifuge::protocol::Reply reply;
				reply.ParseFromArray( b.ptr + b.len, s );
				AS_LOG_DEBUG_LINE(
					"Reply: has error: " << reply.has_error() << " has subscribe: " << reply.has_subscribe()
										 << " has push: " << reply.has_push() << " has ping: " << reply.has_ping() );

				if ( reply.has_error() ) {
					AS_LOG_ERROR_LINE( reply.error().message() );
				}
				else if ( reply.has_connect() ) {
					AS_LOG_DEBUG_LINE( "Connect" );
					OnConnect( *this );
				}
				else if ( reply.has_subscribe() ) {
					AS_LOG_DEBUG_LINE( "Subscribed" );
				}
				else if ( reply.has_push() ) {
					AS_LOG_DEBUG_LINE(
						"Push: has message: " << reply.push().has_message() << " has pub: " << reply.push().has_pub() );

					if ( reply.push().has_pub() ) {
						AS_LOG_DEBUG_LINE( "Pub: channel: " << reply.push().pub().channel()
															<< " data: " << reply.push().pub().data() );

						OnPub( *this, reply.push().pub().channel(), reply.push().pub().data() );
					}
				}

				size -= s + b.len;
				b.ptr += s + b.len;
				b.len = size;
			}
			while ( size > 0 );
		}
		catch ( const std::exception & e ) {
			AS_LOG_ERROR_LINE( e.what() );
		}
		catch ( ... ) {
			AS_LOG_ERROR_LINE( "Unknown exception" );
		}

		return true;
	}


	void CentrifugeClientBase::subscribe( const as::t_stringview channel )
	{
		centrifugal::centrifuge::protocol::Command command;
		command.set_id( m_messageId.fetch_add( 1 ) );
		auto sub = command.mutable_subscribe();
		sub->set_channel( channel.data() );
		std::string message;
		serialize( message, command );
		m_wsClient->write( message.data(), message.size() );
	}

} // namespace as
