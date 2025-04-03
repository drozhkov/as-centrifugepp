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

/// centrifugeClient.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CENTRIFUGEPP__CENTRIFUGE_CLIENT__H
#define __CENTRIFUGEPP__CENTRIFUGE_CLIENT__H


#include <string_view>

#include "logger.hpp"
#include "wsClient.hpp"


namespace as {

	class CentrifugeClientBase {
	protected:
		std::unique_ptr<as::WsClientBase> m_wsClient;
		t_timespan m_wsTimeoutMs{ 0 };

		std::atomic_uint32_t m_messageId{ 1 };

	protected:
		virtual t_string Token() = 0;
		virtual void OnConnect( CentrifugeClientBase & client ) = 0;
		virtual void OnPub(
			CentrifugeClientBase & client, const std::string_view channelName, const std::string_view data ) = 0;

		void wsErrorHandler( as::WsClientBase & client, int code, const as::t_stringview & message )
		{
			AS_LOG_ERROR_LINE( client.Id() << " " << code << ": " << message );
		}


		void wsHandshakeHandler( as::WsClientBase & client );
		bool wsReadHandler( as::WsClientBase & client, const char * data, size_t size );

	public:
		template <typename T> static void serialize( std::string & message, const T & command )
		{
			message.assign( command.SerializeAsString() );

			if ( !message.empty() ) {
				uint8_t sizeBuffer[16];
				as::t_buffer b( sizeBuffer, sizeof sizeBuffer );
				auto size = message.size();
				encodeLength( b, size );

				message.insert( 0, b.len, 0 );

				for ( size_t j = 0; j < b.len; ++j ) {
					message[j] = sizeBuffer[j];
				}
			}
		}


		static void encodeLength( t_buffer & sizeBuffer, size_t & size )
		{
			size_t i = 0;

			while ( size >= 0x80 ) {
				sizeBuffer.ptr[i] = ( size & 0x7f ) | 0x80;
				size >>= 7;
				++i;
			}

			sizeBuffer.ptr[i] = static_cast<t_byte>( size );
			sizeBuffer.len = i + 1;
		}


		static size_t decodeLength( t_buffer & sizeBuffer )
		{
			size_t size = 0;
			size_t shift = 0;

			for ( size_t i = 0; i < sizeBuffer.len; ++i ) {
				size |= static_cast<size_t>( sizeBuffer.ptr[i] & 0x7f ) << shift;
				shift += 7;

				if ( ( sizeBuffer.ptr[i] & 0x80 ) == 0 ) {
					sizeBuffer.len = i + 1;
					break;
				}
			}

			return size;
		}


		void subscribe( const as::t_stringview channel );
	};


	template <typename T_tokenFunc, typename T_connectHandler, typename T_pubHandler>
	class CentrifugeClient : public CentrifugeClientBase {
	protected:
		t_string m_wsUrl;
		T_tokenFunc m_tokenFunc;
		T_connectHandler m_connectHandler;
		T_pubHandler m_pubHandler;

	protected:
		t_string Token() override
		{
			return m_tokenFunc();
		}


		void OnConnect( CentrifugeClientBase & client ) override
		{
			m_connectHandler( client );
		}


		void OnPub(
			CentrifugeClientBase & client, const std::string_view channelName, const std::string_view data ) override
		{

			m_pubHandler( client, channelName, data );
		}


		void initWsClient()
		{
			m_wsClient.reset( new as::WsClient(
				Url( m_wsUrl ),
				[this]( as::WsClientBase & client, int code, const as::t_stringview message ) {
					wsErrorHandler( client, code, message );
				},
				[this]( as::WsClientBase & client ) { wsHandshakeHandler( client ); },
				[this]( as::WsClientBase & client, const char * data, size_t size ) {
					return wsReadHandler( client, data, size );
				} ) );

			m_wsClient->WatchdogTimeoutMs( m_wsTimeoutMs );
		}

	public:
		CentrifugeClient( const as::t_stringview wsUrl,
			const T_tokenFunc & tokenFunc,
			const T_connectHandler & connectHandler,
			const T_pubHandler & pubHandler )
			: m_wsUrl( wsUrl )
			, m_tokenFunc( tokenFunc )
			, m_connectHandler( connectHandler )
			, m_pubHandler( pubHandler )
		{
		}


		void run()
		{
			std::thread t( [this] {
				size_t wsClientId = 1;

				while ( true ) {
					try {
						initWsClient();
						m_wsClient->Id( std::to_string( wsClientId++ ) );
						m_wsClient->run();
					}
					catch ( const std::exception & x ) {
						AS_LOG_ERROR_LINE( x.what() );
					}
				}
			} );

			t.join();
		}
	};

} // namespace as


#endif
