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
#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif
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
	// @@@ Currently, it only supports IPv4
	struct sockaddr_in _socket_sockaddr;
	socklen_t _endpoint_sockaddr_len{};

      public:
	Socket(std::string IP_addr, std::uint16_t port, int backlog,
	       Domain communication_domain, SockType socket_type);
	Socket(const Socket &) = delete;
	Socket(Socket &&);
	Socket() = default;
	Socket &operator=(Socket &&);
	void setSocketOption(SockOptLevel sock_level,
			     SocketOptions sock_opt) noexcept;
	const std::string &getIP() const noexcept;
	const std::uint16_t &getPort() const noexcept;
	const int getSocketBacklog() const noexcept;
	const Domain getSocketDomain() const noexcept;
	const int getFileDescriptor() const noexcept;
	int acceptOnce();
	// Possable we can replace `char*` with an implementation of a IOBuffer
	// object in future
	void readBuffer(char *read_buffer, std::size_t max_read_buff);
	void writeBuffer(const char *write_buffer, std::size_t write_size);
	void bindSock() noexcept;
	void makeSocketNonBlocking() noexcept(false);
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
	std::swap(_socket_sockaddr, tcp_endpoint._socket_sockaddr);
	std::swap(_endpoint_sockaddr_len, tcp_endpoint._endpoint_sockaddr_len);
	return *this;
}

inline Socket::Socket(Socket &&tcp_endpoint) {
	std::swap(_IP_addr, tcp_endpoint._IP_addr);
	std::swap(_port, tcp_endpoint._port);
	std::swap(_endpoint_backlog, tcp_endpoint._endpoint_backlog);
	std::swap(_domain, tcp_endpoint._domain);
	std::swap(_file_des, tcp_endpoint._file_des);
	std::swap(_sock_type, tcp_endpoint._sock_type);
	std::swap(_socket_sockaddr, tcp_endpoint._socket_sockaddr);
	std::swap(_endpoint_sockaddr_len, tcp_endpoint._endpoint_sockaddr_len);
}

inline void Socket::m_listen_socket() {
	int ret_code = ::listen(_file_des, _endpoint_backlog);
	err_check(ret_code, "linux listen()");
}

inline int Socket::acceptOnce() {
	return ::accept(_file_des, (sockaddr *)&_socket_sockaddr,
			&_endpoint_sockaddr_len);
}

inline Socket::Socket(std::string IP_addr, std::uint16_t port, int backlog,
		      Domain communication_domain, SockType socket_type)
    : _IP_addr{std::move(IP_addr)}, _port{port}, _endpoint_backlog{backlog},
      _domain{communication_domain}, _sock_type{socket_type} {
	m_create_socket();
}

inline void Socket::setSocketOption(SockOptLevel sock_level,
				    SocketOptions sock_opt) noexcept {
	int optval = 1;
	int ret_code =
	    ::setsockopt(_file_des, static_cast<int>(sock_level),
			 static_cast<int>(sock_opt), &optval, sizeof(optval));
	err_check(ret_code, "linux setsockopt() err");
}

inline void Socket::m_create_socket() {
	std::memset(&_socket_sockaddr, 0, sizeof(sockaddr_in));
	_socket_sockaddr.sin_family = static_cast<int>(_domain);
	inet_pton(static_cast<int>(_domain), _IP_addr.c_str(),
		  &_socket_sockaddr.sin_addr);
	_socket_sockaddr.sin_port = htons(_port);
	_file_des = ::socket(static_cast<int>(_domain),
			     static_cast<int>(_sock_type), 0);
	err_check(_file_des, "socket() creation");
	_endpoint_sockaddr_len = sizeof(_socket_sockaddr);
}

inline void Socket::bindSock() noexcept {
	int ret_code =
	    ::bind(_file_des, reinterpret_cast<sockaddr *>(&_socket_sockaddr),
		   sizeof(sockaddr_in));
	err_check(ret_code, "bind() error");
	m_listen_socket();
}

inline void Socket::readBuffer(char *read_buffer, std::size_t max_read_buff) {
	// TODO: implementation of a IOBuffer object for handling the raw bytes
	// into iteratable object. Currently in Blocking IO
	//
	// UPDATE: We use a 'async-stream' and 'sync-stream' whose
	// implementation we use the IOBUffer
	read_data(_file_des, read_buffer, max_read_buff, 0);
}

inline void Socket::writeBuffer(const char *write_buffer,
				std::size_t write_size) {
	write_data(_file_des, write_buffer, write_size, 0);
}

inline const std::string &Socket::getIP() const noexcept { return _IP_addr; }

inline const std::uint16_t &Socket::getPort() const noexcept { return _port; }

inline const int Socket::getSocketBacklog() const noexcept {
	return _endpoint_backlog;
}

inline const Domain Socket::getSocketDomain() const noexcept { return _domain; }

inline const int Socket::getFileDescriptor() const noexcept {
	return _file_des;
}

inline void Socket::makeSocketNonBlocking() noexcept(false) {
	int flags = ::fcntl(_file_des, F_GETFL, 0);
	if (flags == -1) { throw std::runtime_error("fnctl() F_GETFL"); }
	int ret = ::fcntl(_file_des, F_SETFL, flags | O_NONBLOCK);
	if (ret == -1) { throw std::runtime_error("fnctl() O_NONBLOCK"); }
}

inline Socket::~Socket() { ::close(_file_des); }

static void makeSocketNonBlocking(int socket) noexcept(false) {
	int flags = ::fcntl(socket, F_GETFL, 0);
	if (flags == -1) { throw std::runtime_error("fnctl() F_GETFL"); }
	int ret = ::fcntl(socket, F_SETFL, flags | O_NONBLOCK);
	if (ret == -1) { throw std::runtime_error("fcntl() O_NONBLOCK"); }
}

} // namespace blueth::net
