#pragma once
#include "http/HTTPConstants.hpp"
#include "http/HTTPHeaders.hpp"
#include "http/HTTPParserStateMachine.hpp"
#include "net/SyncNetworkStreamClient.hpp"
#include "utils/Base64.hpp"
#include <exception>
#include <optional>

namespace blueth::http {

enum class HTTPProxyReturnCode {
	ProxyAuthRequired,
	ConnectionSuccess,
	NoProxySupport,
	AuthFailed
};

class HTTPProxyClient {
      public:
	/**
	 * Takes a network handler 'net::NetworkStream' object which already
	 * have a open connection to the desired HTTP Proxy to send/recv raw
	 * bytes Since we can transparently place SSL/Plain-text network
	 * clients, we don't have to bother with if the HTTP Proxy is over SSL
	 * or Plain-text since it's an interface
	 *
	 * @param network_handler Underlying networking handler which have an
	 * open connection.
	 */
	HTTPProxyClient(
	    std::unique_ptr<net::NetworkStream<char>> network_handler);
	/**
	 * Make a HTTP Proxy Conenct request to the HTTP Proxy endpoint and
	 * return the status. If the connection to the origin-server is
	 * successful, HTTP 2xx message is returned by the HTTP Proxy,
	 * indicating that we now can tunnel the request bytes
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
	 *
	 * HTTPProxyErrorCode::NoProxySupport:
	 * 	Means, the HTTP Proxy endpoint doesn't implement the HTTP
	 * CONNECT tunnel requet
	 *
	 * HTTPProxyErrorCode::AuthFailed:
	 * 	Means, the username or password is invalid.
	 *
	 * @param origin_server The Origin-Server to connect to through this
	 * HTTP Proxy
	 * @param proxy_username Optional value for username for the HTTP Proxy
	 * for authentication
	 * @param proxy_passphrase Optional value for passphrase for the HTTP
	 * Proxy for authentication
	 * @return Return value of type HTTPProxyErrorCode
	 */
	HTTPProxyReturnCode
	makeConnection(std::string origin_server,
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
	 */
	void writeProxy(const std::string &data) noexcept(false);
	/**
	 * Get the Origin server we are connected to through this HTTP Proxy or
	 * trying to connect to(or failed)
	 *
	 * @return origin server's hostname
	 */
	std::string getOriginServer() const noexcept;
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

} // namespace blueth::http
