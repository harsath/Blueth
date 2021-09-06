#pragma once
#include "concurrency/internal/EventLoopBase.hpp"
#include "http/HTTPMessage.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace blueth::http {
// TODO: add TLS support, currently it only supports plaintext-HTTP
template <typename PeerState> class HTTPMux {
	using HandlerCallbackType = std::function<void(
	    HTTPRequestMessage,
	    std::shared_ptr<concurrency::EventLoopBase<PeerState>>)>;

      public:
	/**
	 * Initilize a new instance of HTTP Multiplexer or Router.
	 *
	 * @param listen_address IPv4 address that the handler will listen
	 * @param port Port number to listen on
	 */
	HTTPMux(const std::string &listen_address, std::uint16_t port);
	/**
	 * Callback function to be invoked when a request to a particular
	 * handler is being made
	 *
	 * @param endpoint The HTTP request's endpoint
	 * @param The callback handler that need to be invoked when a request
	 * comes to this particular endpoint
	 */
	void addHandler(std::string endpoint, HandlerCallbackType callback);
	/**
	 * Listen on the given IPv4 address and port with registered handlers
	 */
	void startAndListen();

      private:
	std::unordered_map<
	    std::string, std::shared_ptr<concurrency::EventLoopBase<PeerState>>>
	    endpoint_fn_ptr_map_;
	bool is_ssl_;
	std::string ssl_cert_path_;
	std::string ssl_priv_path_;
};
} // namespace blueth::http
