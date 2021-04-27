#pragma once
#include "HTTPMessage.hpp"
#include "http/HTTPConstants.hpp"
#include "http/HTTPHeaders.hpp"
#include "http/HTTPParserStateMachineResponse.hpp"
#include "io/IOBuffer.hpp"
#include "net/NetworkStream.hpp"
#include "net/SyncNetworkStreamClient.hpp"
#include "utils/Base64.hpp"
#include <cstdint>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>

namespace blueth::http {

enum class HTTPProxyReturnCode {
	ProxyAuthRequired,
	ConnectionSuccess,
	NoProxySupport,
	AuthFailed,
	NetworkError,
	InvalidResponse
};

class HTTPProxyClient {
	std::unique_ptr<net::NetworkStream<char>> network_handler_;
	std::string origin_server_hostname_;
	std::uint16_t origin_server_port_;
	std::optional<std::string> proxy_username_;
	std::optional<std::string> proxy_passphrase_;
	bool proxy_connection_made_{false};

      public:
	/**
	 * Takes a network handler 'net::NetworkStream' object which already
	 * have a open connection to the desired HTTP Proxy server to send/recv
	 * raw bytes. Since we can transparently place SSL/Plain-text network
	 * clients, we don't have to bother with if the HTTP Proxy server is
	 * connected over SSL or Plain-text since it's something taken care for
	 * use by the 'net::NetworkStream' interface
	 *
	 * @param network_handler Underlying networking handler which have an
	 * open connection.
	 */
	HTTPProxyClient(
	    std::unique_ptr<net::NetworkStream<char>> network_handler);
	/**
	 * Make a HTTP Proxy CONNECT request to the HTTP Proxy server endpoint,
	 * parse the Proxy's response message and return the status. If the
	 * connection to the origin-server is successful, HTTP 2xx message is
	 * returned by the HTTP Proxy, indicating that we now can tunnel the
	 * bytes sent from the 'http::HTTPProxyClient' interface
	 *
	 * Optionally, the interface also supports HTTP Proxy Authentication to
	 * establish the authority to create a tunnel through the Proxy The
	 * method returns a HTTPProxyErrorCode type, which indicates the HTTP
	 * CONNECT and Authentication's status
	 *
	 * HTTPProxyErrorCode::ProxyAuthRequired:
	 * 	Means, the HTTP Proxy we are trying to connect/create a tunnel
	 * through requires authentication.
	 *
	 * HTTPProxyErrorCode::ConnectionSuccess:
	 * 	Means, we created a HTTP Tunnel through the Proxy successfully.
	 * we can send/recv bytes through the HTTP Proxy server to the
	 * origin-server endpoint
	 *
	 * HTTPProxyErrorCode::NoProxySupport:
	 * 	Means, the HTTP Proxy endpoint doesn't implement the HTTP
	 * CONNECT tunnel request
	 *
	 * HTTPProxyErrorCode::AuthFailed:
	 * 	Means, the username or password is invalid.
	 *
	 * @param origin_server_hostname The Origin-Server host to connect to
	 * through this HTTP Proxy
	 * @param origin_server_port Origin server's port to connect to, must be
	 * an endpoint which talks HTTP
	 * @param proxy_username Optional value for username for the HTTP Proxy
	 * for authentication
	 * @param proxy_passphrase Optional value for passphrase for the HTTP
	 * Proxy for authentication
	 * @return Return value of type HTTPProxyErrorCode
	 */
	HTTPProxyReturnCode
	makeConnection(std::string origin_server_hostname,
		       std::uint16_t origin_server_port,
		       std::optional<std::string> proxy_username,
		       std::optional<std::string> proxy_passphrase);
	/**
	 * Read bytes through the HTTP Proxy as a tunnel between the client and
	 * the connected origin-server
	 *
	 * If the connection is failed or there is no connection to the origin
	 * server established through the HTTP Proxy in the first place, a
	 * std::runtime_error is thrown
	 *
	 * @param read_length Number of bytes to read from the proxy
	 * @return std::string representation of the raw bytes read from through
	 * the HTTP Proxy tunnel from the Origin server
	 */
	std::string readProxy(size_t read_length) noexcept(false);
	/**
	 * Write some bytes to the Origin server through the tunnel between HTTP
	 * Proxy and the client.
	 *
	 * If the connection is failed or there is no connection to the origin
	 * server established through the HTTP Proxy in the first place, a
	 * std::runtime_error is thrown
	 *
	 * @param data std::string representation of the raw byte to write the
	 * proxy
	 * @return Returns number of bytes written on success and negative value
	 * on a failue. We assume that the user have a logic in-place to handle
	 * the write error in a graceful way.
	 */
	template <typename StringType>
	int writeProxy(StringType &&data) noexcept(false);
	/**
	 * Get the Origin server hostname we are connected to through this HTTP
	 * Proxy or trying to connect to(or failed)
	 *
	 * @return origin server's hostname
	 */
	std::string getOriginServerHost() const noexcept;
	/**
	 * Get the Origin server's port number we are connected to.
	 *
	 * @return origin server's port number
	 */
	std::uint16_t getOriginServerPort() const noexcept;
	/**
	 * Check if the connection is made to the HTTP Proxy so that the bytes
	 * written on the wire are tunned through the Proxy server to the origin
	 * server
	 *
	 * @return Boolean flag indicating if the connection made or not
	 */
	bool connectedToProxy() const noexcept;
	/**
	 * Get the Proxy's username info, if we had specified it in the past for
	 * the authentication of the HTTP proxy
	 *
	 * @return Optional username auth info of the Proxy
	 */
	std::optional<std::string> getProxyUsername() const noexcept;
	/**
	 * Get the Proxy's passphrase info, if we had specified it in the past
	 * for the authentication of the HTTP Proxy
	 *
	 * @return Optional passphrase auth info of the Proxy
	 */
	std::optional<std::string> getProxyPassphrase() const noexcept;
	~HTTPProxyClient() = default;
};

HTTPProxyClient::HTTPProxyClient(
    std::unique_ptr<net::NetworkStream<char>> network_handler)
    : network_handler_{std::move(network_handler)} {}

HTTPProxyReturnCode HTTPProxyClient::makeConnection(
    std::string origin_server_hostname, std::uint16_t origin_server_port,
    std::optional<std::string> proxy_username = std::nullopt,
    std::optional<std::string> proxy_passphrase = std::nullopt) {
	origin_server_hostname_ = std::move(origin_server_hostname);
	origin_server_port_ = origin_server_port;
	proxy_username_ = std::move(proxy_username);
	proxy_passphrase_ = std::move(proxy_passphrase);
	std::string hostname_and_port =
	    origin_server_hostname_ + ":" + std::to_string(origin_server_port_);
	HTTPRequestMessage connect_request_message;
	connect_request_message.setRequestType(HTTPRequestType::Connect);
	connect_request_message.setHTTPTargetResource(hostname_and_port);
	connect_request_message.setHTTPVersion(HTTPVersion::HTTP1_1);
	connect_request_message.addHeader("Host", hostname_and_port);
	connect_request_message.addHeader("User-Agent", "blueth/http-client");
	connect_request_message.addHeader("Proxy-Connection", "Keep-Alive");
	if (proxy_username_ != std::nullopt &&
	    proxy_passphrase_ != std::nullopt) {
		std::string username_passphrase_b64 =
		    proxy_username_.value() + ":" + proxy_passphrase_.value();
		connect_request_message.addHeader(
		    "Proxy-Authorization",
		    "Basic " + base64::base64_encode(
				   (const unsigned char *)
				       username_passphrase_b64.c_str(),
				   username_passphrase_b64.size()));
	}
	int write_ret = network_handler_->streamWrite(
	    connect_request_message.buildRawMessage());
	if (write_ret < 0) return HTTPProxyReturnCode::NetworkError;
	net::NetworkStream<char>::const_buffer_reference_type proxy_response =
	    network_handler_->constGetIOBuffer();
	std::unique_ptr<HTTPResponseMessage> http_response_message =
	    HTTPResponseMessage::create();
	ResponseParserState current_state =
	    ResponseParserState::ResponseProtocolH;
	std::unique_ptr<HTTPResponseMessage> parsed_message =
	    ParseHTTP1_1ResponseMessage(proxy_response, current_state,
					std::move(http_response_message));
	if (current_state == ResponseParserState::ProtocolError)
		return HTTPProxyReturnCode::InvalidResponse;
	if (current_state == ResponseParserState::ParsingDone) {
		if (parsed_message->getResponseCode() ==
		    HTTPResponseCodes::Unauthorized)
			return HTTPProxyReturnCode::ProxyAuthRequired;
		if (parsed_message->getResponseCode() ==
		    HTTPResponseCodes::Ok) {
			proxy_connection_made_ = true;
			return HTTPProxyReturnCode::ConnectionSuccess;
		} else {
			return HTTPProxyReturnCode::NoProxySupport;
		}
	} else {
		return HTTPProxyReturnCode::InvalidResponse;
	}
}

std::string HTTPProxyClient::readProxy(size_t read_length) noexcept(false) {
	if (!proxy_connection_made_)
		throw std::runtime_error{
		    "No proxy connection to the origin-server"};
	int ret = network_handler_->streamRead(read_length);
	if (ret < 0) return "";
	std::string returner{
	    network_handler_->constGetIOBuffer()->getStartOffsetPointer(),
	    network_handler_->constGetIOBuffer()->getEndOffsetPointer()};
	net::NetworkStream<char>::buffer_type io_buffer =
	    network_handler_->getIOBuffer();
	// Currently we relay on blocking-read/write through the network
	io_buffer->clear();
	network_handler_->setIOBuffer(std::move(io_buffer));
	return returner;
}

template <typename StringType>
int HTTPProxyClient::writeProxy(StringType &&data) noexcept(false) {
	if (!proxy_connection_made_)
		throw std::runtime_error{
		    "No proxy connection to the origin-server"};
	return network_handler_->streamWrite(std::forward<StringType>(data));
}

std::string HTTPProxyClient::getOriginServerHost() const noexcept {
	return origin_server_hostname_;
}

std::uint16_t HTTPProxyClient::getOriginServerPort() const noexcept {
	return origin_server_port_;
}

bool HTTPProxyClient::connectedToProxy() const noexcept {
	return proxy_connection_made_;
}

std::optional<std::string> HTTPProxyClient::getProxyUsername() const noexcept {
	return proxy_username_;
}

std::optional<std::string>
HTTPProxyClient::getProxyPassphrase() const noexcept {
	return proxy_passphrase_;
}

} // namespace blueth::http
