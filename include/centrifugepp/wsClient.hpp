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

/// wsClient.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CENTRIFUGEPP__WS_CLIENT__H
#define __CENTRIFUGEPP__WS_CLIENT__H


#include <mutex>
#include <atomic>
#include <thread>
#include <string_view>

#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "boost/beast/core.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/beast/websocket/ssl.hpp"

#include "core.hpp"
#include "logger.hpp"
#include "url.hpp"


namespace as {

	using t_wsStream = boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;


	class WsClientBase {
	protected:
		t_timespan m_watchdogTimeoutMs = 15 * 1000;

		Url m_url;

		boost::asio::io_context m_io;
		boost::asio::ssl::context m_ctx;

		t_wsStream m_stream;

		std::mutex m_streamWriteSync;
		std::mutex m_streamPingSync;

		boost::beast::flat_buffer m_buffer;

		std::atomic_int64_t m_lastActivityTs;

		std::thread m_watchdogThread;
		std::atomic_flag m_isWatchdogActive;

		t_string m_id;

	protected:
		static auto NowTs()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch() )
				.count();
		}


		void refreshLastActivityTs()
		{
			m_lastActivityTs.store( NowTs() );
		}


		virtual void OnResolve(
			boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results ) = 0;

		virtual void OnConnect(
			boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type::iterator it ) = 0;

		virtual void OnSslHandshake( boost::system::error_code ec ) = 0;
		virtual void OnHandshake( boost::system::error_code ec ) = 0;
		virtual void OnWriteComplete( boost::system::error_code ec, std::size_t bytesWritten ) = 0;
		virtual void OnReadComplete( boost::system::error_code ec, std::size_t bytesRead ) = 0;
		virtual void OnPingComplete( boost::system::error_code ec ) = 0;
		virtual void OnControl( boost::beast::websocket::frame_type, boost::beast::string_view ) = 0;
		virtual void OnClose( boost::system::error_code ec ) = 0;

	public:
		WsClientBase( const Url & url )
			: m_url( url )
			, m_ctx( boost::asio::ssl::context::method::tls_client )
			, m_stream( m_io, m_ctx )
		{

			m_stream.binary( true );
		}


		virtual ~WsClientBase()
		{
			m_isWatchdogActive.clear();

			if ( m_watchdogThread.joinable() ) {
				m_watchdogThread.join();
			}
		}


		void WatchdogTimeoutMs( t_timespan t )
		{
			if ( t != 0 ) {
				m_watchdogTimeoutMs = t;
			}
		}


		bool IsOpen() const
		{
			return m_stream.is_open();
		}


		const t_string & Id() const
		{
			return m_id;
		}


		void Id( const t_stringview v )
		{
			m_id = v;
		}


		void run();
		void readAsync();
		bool write( const void * data, size_t size );
		void writeAsync( const void * data, size_t size );
		void pingAsync( const void * data, size_t size );
	};


	template <typename T_errorHandler, typename T_handshakeHandler, typename T_readHandler>
	class WsClient : public WsClientBase {
	protected:
		T_errorHandler m_errorHandler;
		T_handshakeHandler m_handshakeHandler;
		T_readHandler m_readHandler;

	protected:
		void OnResolve( boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results ) override
		{
			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}

			boost::asio::async_connect( m_stream.next_layer().next_layer(),
				results.begin(),
				results.end(),
				std::bind( &WsClient::OnConnect, this, std::placeholders::_1, std::placeholders::_2 ) );
		}


		void OnConnect(
			boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type::iterator it ) override
		{

			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}

			SSL_set_tlsext_host_name( m_stream.next_layer().native_handle(), m_url.Hostname().c_str() );

			m_stream.next_layer().async_handshake( boost::asio::ssl::stream_base::client,
				std::bind( &WsClient::OnSslHandshake, this, std::placeholders::_1 ) );
		}


		void OnSslHandshake( boost::system::error_code ec ) override
		{
			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}

			m_stream.set_option(
				boost::beast::websocket::stream_base::decorator( []( boost::beast::websocket::request_type & req ) {
					req.set( boost::beast::http::field::sec_websocket_protocol, "centrifuge-protobuf" );
				} ) );

			m_stream.async_handshake(
				m_url.Hostname(), m_url.Path(), std::bind( &WsClient::OnHandshake, this, std::placeholders::_1 ) );
		}


		void OnHandshake( boost::system::error_code ec ) override
		{
			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}

			m_stream.control_callback(
				std::bind( &WsClient::OnControl, this, std::placeholders::_1, std::placeholders::_2 ) );

			m_handshakeHandler( *this );
		}


		void OnWriteComplete( boost::system::error_code ec, std::size_t bytesWritten ) override
		{
			boost::ignore_unused( bytesWritten );

			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}
		}


		void OnReadComplete( boost::system::error_code ec, std::size_t bytesRead ) override
		{
			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}

			refreshLastActivityTs();

			try {
				if ( !m_readHandler(
						 *this, static_cast<const char *>( m_buffer.data().data() ), m_buffer.data().size() ) ) {

					return;
				}
			}
			catch ( ... ) {
			}

			m_buffer.consume( bytesRead );

			readAsync();
		}


		void OnPingComplete( boost::system::error_code ec ) override
		{
			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}
		}


		void OnClose( boost::system::error_code ec ) override
		{
			if ( ec ) {
				m_errorHandler( *this, ec.value(), ec.message() );
				return;
			}
		}


		void OnControl( boost::beast::websocket::frame_type type, boost::beast::string_view payload ) override
		{
			AS_LOG_TRACE_LINE( payload );

			refreshLastActivityTs();

			if ( boost::beast::websocket::frame_type::ping == type ) {
				m_stream.async_pong( "as::wsClient", []( boost::system::error_code ) {} );
			}
		}

	public:
		WsClient( const Url & url,
			const T_errorHandler & errorHandler,
			const T_handshakeHandler & handshakeHandler,
			const T_readHandler & readHandler )
			: WsClientBase( url )
			, m_errorHandler( errorHandler )
			, m_handshakeHandler( handshakeHandler )
			, m_readHandler( readHandler )
		{
		}
	};

} // namespace as


#endif
