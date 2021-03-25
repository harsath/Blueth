#pragma once
#include "TransportHelpers.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace blueth::net {
namespace Transport {
#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif
using namespace blueth::net::TransportHelper;
enum class Domain : int { Unix = AF_UNIX, Ipv4 = AF_INET, Ipv6 = AF_INET6 };
enum class SockType : int { Stream = SOCK_STREAM, Datagram = SOCK_DGRAM };
enum class SockOptLevel : int { SocketLevel = SOL_SOCKET, TcpLevel = SOL_TCP };
enum class SocketOptions : int {
	ReuseAddress = SO_REUSEADDR,
	ReusePort = SO_REUSEPORT,
	TcpNoDelay = TCP_NODELAY
}; // currently supported Opts
class Socket {
      private:
	std::string _IP_addr;
	std::uint16_t _port{};
	int _endpoint_backlog{};
	Domain _domain;
	int _file_des{};
	SockType _sock_type;
	struct sockaddr_in _server_sockaddr;
	int _endpoint_sock_len{};

      public:
	Socket(const std::string &IP_addr, const std::uint16_t &port,
	       int backlog, Domain communication_domain, SockType socket_type);
	Socket(const Socket &) = delete;
	Socket(Socket &&);
	Socket() = default;
	Socket &operator=(Socket &&);
	void set_socket_options(SockOptLevel sock_level,
				SocketOptions sock_opt) noexcept;
	const std::string &get_ip() const noexcept;
	const std::uint16_t &get_port() const noexcept;
	const int get_socket_backlog() const noexcept;
	const Domain get_socket_domain() const noexcept;
	const int get_file_descriptor() const noexcept;
	const int accept_loop();
	// Possable we can replace `char*` with an implementation of a IOBuffer
	// object in future
	void read_buff(char *read_buffer, std::size_t max_read_buff);
	void write_buff(const char *write_buffer, std::size_t write_size);
	void bind_sock() noexcept;
	void make_socket_nonblocking();
	void close_serving_client_connection() noexcept;
	std::string get_serving_client_ip() noexcept;

	~Socket();

      protected:
	void m_create_socket();
	void m_listen_socket();
};

inline Socket &Socket::operator=(Socket &&tcp_endpoint) {
	std::swap(_IP_addr, tcp_endpoint._IP_addr);
	std::swap(_port, tcp_endpoint._port);
	std::swap(_endpoint_backlog, tcp_endpoint._endpoint_backlog);
	std::swap(_domain, tcp_endpoint._domain);
	std::swap(_file_des, tcp_endpoint._file_des);
	std::swap(_sock_type, tcp_endpoint._sock_type);
	std::swap(_server_sockaddr, tcp_endpoint._server_sockaddr);
	std::swap(_endpoint_sock_len, tcp_endpoint._endpoint_sock_len);
	return *this;
}

inline Socket::Socket(Socket &&tcp_endpoint) {
	std::swap(_IP_addr, tcp_endpoint._IP_addr);
	std::swap(_port, tcp_endpoint._port);
	std::swap(_endpoint_backlog, tcp_endpoint._endpoint_backlog);
	std::swap(_domain, tcp_endpoint._domain);
	std::swap(_file_des, tcp_endpoint._file_des);
	std::swap(_sock_type, tcp_endpoint._sock_type);
	std::swap(_server_sockaddr, tcp_endpoint._server_sockaddr);
	std::swap(_endpoint_sock_len, tcp_endpoint._endpoint_sock_len);
}

inline void Socket::m_listen_socket() {
	int ret_code = ::listen(_file_des, _endpoint_backlog);
	err_check(ret_code, "linux listen()");
}

inline Socket::Socket(const std::string &IP_addr, const std::uint16_t &port,
		      int backlog, Domain communication_domain,
		      SockType socket_type)
    : _IP_addr{IP_addr}, _port{port}, _endpoint_backlog{backlog},
      _domain{communication_domain}, _sock_type{socket_type} {
	m_create_socket();
}

inline void Socket::set_socket_options(SockOptLevel sock_level,
				       SocketOptions sock_opt) noexcept {
	int optval = 1;
	int ret_code =
	    ::setsockopt(_file_des, static_cast<int>(sock_level),
			 static_cast<int>(sock_opt), &optval, sizeof(optval));
	err_check(ret_code, "linux setsockopt() err");
}

inline void Socket::m_create_socket() {
	std::memset(&_server_sockaddr, 0, sizeof(sockaddr_in));
	_server_sockaddr.sin_family = static_cast<int>(_domain);
	inet_pton(static_cast<int>(_domain), _IP_addr.c_str(),
		  &_server_sockaddr.sin_addr);
	_server_sockaddr.sin_port = htons(_port);
	_file_des = ::socket(static_cast<int>(_domain),
			     static_cast<int>(_sock_type), 0);
	err_check(_file_des, "socket() creation");
	_endpoint_sock_len = sizeof(_server_sockaddr);
}

inline void Socket::bind_sock() noexcept {
	int ret_code =
	    ::bind(_file_des, reinterpret_cast<sockaddr *>(&_server_sockaddr),
		   sizeof(sockaddr_in));
	err_check(ret_code, "bind() error");
	m_listen_socket();
}

inline void Socket::read_buff(char *read_buffer, std::size_t max_read_buff) {
	// TODO: implementation of a IOBuffer object for handling the raw bytes
	// into iteratable object. Currently in Blocking IO
	read_data(_file_des, read_buffer, max_read_buff, 0);
}

inline void Socket::write_buff(const char *write_buffer,
			       std::size_t write_size) {
	write_data(_file_des, write_buffer, write_size, 0);
}

inline const std::string &Socket::get_ip() const noexcept { return _IP_addr; }

inline const std::uint16_t &Socket::get_port() const noexcept { return _port; }

inline const int Socket::get_socket_backlog() const noexcept {
	return _endpoint_backlog;
}

inline const Domain Socket::get_socket_domain() const noexcept {
	return _domain;
}

inline const int Socket::get_file_descriptor() const noexcept {
	return _file_des;
}

inline void Socket::make_socket_nonblocking() {
	int flags = ::fcntl(_file_des, F_GETFL, 0);
	if (flags == -1) { throw std::runtime_error("fnctl() F_GETFL"); }
	int ret = ::fcntl(_file_des, F_SETFL, flags | O_NONBLOCK);
	if (ret == -1) { throw std::runtime_error("fnctl() O_NONBLOCK"); }
}

inline Socket::~Socket() { ::close(_file_des); }

} // end namespace Transport
} // end namespace blueth::net
