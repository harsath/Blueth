#pragma once
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "TransportHelpers.hpp"

namespace blueth::net{
namespace Transport{
#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif
	using namespace blueth::net::TransportHelper;
	enum class TransportType : std::uint8_t { TCP };
	enum class Domain { UNIX=AF_UNIX, IPv4=AF_INET, IPv6=AF_INET6 };
	enum class SockType{ Stream=SOCK_STREAM, Datagram=SOCK_DGRAM, NonBlocking=SOCK_NONBLOCK };
	enum class SockOptLevel{ SocketLevel=SOL_SOCKET, TcpLevel=SOL_TCP };
	enum class SocketOptions{ ReuseAddress=SO_REUSEADDR, ReusePort=SO_REUSEPORT, TcpNoDelay=TCP_NODELAY }; // currently supported Opts
	class TCPEndpoint{
		private:
			std::string _IP_addr;
			std::uint16_t _port;
			int _endpoint_backlog;
			TransportType _transport_type = TransportType::TCP;
			Domain _domain;
			int _endpoint_fd;
			SockType _sock_type;
			int _serving_client_fd;
			struct sockaddr_in _server_sockaddr, _client_sockaddr;
			int _endpoint_sock_len;
		public:
			TCPEndpoint(
					const std::string& IP_addr, const std::uint16_t& port, int backlog,
					TransportType transport_type, Domain communication_domain,
					SockType socket_type
				   );
			TCPEndpoint(const TCPEndpoint&) = delete;
		 	TCPEndpoint(TCPEndpoint&&);
			TCPEndpoint() = default;
			TCPEndpoint& operator=(TCPEndpoint&&);
			void SetSocketOption(SockOptLevel sock_level, SocketOptions sock_opt) noexcept;
			const std::string& get_ip(void) const noexcept;
			const std::uint16_t& get_port(void) const noexcept;
			const int get_endpoint_backlog(void) const noexcept;
			const Domain get_socket_domain(void) const noexcept;
			const int get_endpoint_fd(void) const noexcept;
			const int get_serving_client(void) const noexcept;
			const int accept_loop();
			// Possable we can replace `char*` with an implementation of a BufferRead object in future
			void read_buff(char* read_buffer, std::size_t max_read_buff);
	 		void write_buff(const char* write_buffer, std::size_t write_size);
			void bind_sock() noexcept;
			void make_socket_nonblocking(void);
			void close_serving_client_connection(void) noexcept;
			std::string get_serving_client_ip() noexcept;

			~TCPEndpoint();
		protected:
			void m_create_socket();
			void m_listen_socket();
	};
	inline TCPEndpoint& TCPEndpoint::operator=(TCPEndpoint&& tcp_endpoint){
		this->_IP_addr = std::move(tcp_endpoint._IP_addr);
		this->_port = tcp_endpoint._port; tcp_endpoint._port = -1;
		this->_endpoint_backlog = tcp_endpoint._endpoint_backlog; tcp_endpoint._endpoint_backlog = -1;
		this->_transport_type = tcp_endpoint._transport_type; tcp_endpoint._transport_type = static_cast<TransportType>(-1);
		this->_domain = tcp_endpoint._domain; tcp_endpoint._domain = static_cast<Domain>(-1);
		this->_endpoint_fd = tcp_endpoint._endpoint_fd; tcp_endpoint._endpoint_fd = -1;
		this->_sock_type = tcp_endpoint._sock_type; tcp_endpoint._sock_type = static_cast<SockType>(-1);
		this->_serving_client_fd = tcp_endpoint._serving_client_fd; tcp_endpoint._serving_client_fd = -1;
		this->_client_sockaddr = tcp_endpoint._client_sockaddr;
		this->_server_sockaddr = tcp_endpoint._server_sockaddr;
		this->_endpoint_sock_len = tcp_endpoint._endpoint_sock_len; tcp_endpoint._endpoint_sock_len = -1;
		this->m_create_socket();
		return *this;
	}
	inline TCPEndpoint::TCPEndpoint(TCPEndpoint&& tcp_endpoint){
		this->_IP_addr = std::move(tcp_endpoint._IP_addr);
		this->_port = tcp_endpoint._port; tcp_endpoint._port = -1;
		this->_endpoint_backlog = tcp_endpoint._endpoint_backlog; tcp_endpoint._endpoint_backlog = -1;
		this->_transport_type = tcp_endpoint._transport_type; tcp_endpoint._transport_type = static_cast<TransportType>(-1);
		this->_domain = tcp_endpoint._domain; tcp_endpoint._domain = static_cast<Domain>(-1);
		this->_endpoint_fd = tcp_endpoint._endpoint_fd; tcp_endpoint._endpoint_fd = -1;
		this->_sock_type = tcp_endpoint._sock_type; tcp_endpoint._sock_type = static_cast<SockType>(-1);
		this->_serving_client_fd = tcp_endpoint._serving_client_fd; tcp_endpoint._serving_client_fd = -1;
		this->_client_sockaddr = tcp_endpoint._client_sockaddr;
		this->_server_sockaddr = tcp_endpoint._server_sockaddr;
		this->_endpoint_sock_len = tcp_endpoint._endpoint_sock_len; tcp_endpoint._endpoint_sock_len = -1;
		this->m_create_socket();
	}
	inline void TCPEndpoint::m_listen_socket(){
		int ret_code = ::listen(this->_endpoint_fd, this->_endpoint_backlog);
		err_check(ret_code, "linux listen()");
	}
	inline TCPEndpoint::TCPEndpoint(
					const std::string& IP_addr, const std::uint16_t& port, int backlog,
					TransportType transport_type, Domain communication_domain,
					SockType socket_type
			) : _IP_addr{IP_addr}, _port{port}, _endpoint_backlog{backlog}, _transport_type{transport_type}, _domain{communication_domain},
	       _sock_type{socket_type} {
		       this->m_create_socket();
	       }
	inline void TCPEndpoint::SetSocketOption(SockOptLevel sock_level, SocketOptions sock_opt) noexcept {
		int optval = 1;
		int ret_code = ::setsockopt(this->_endpoint_fd, static_cast<int>(sock_level), static_cast<int>(sock_opt), &optval, sizeof(optval));
		err_check(ret_code, "linux setsockopt() err");
	}
	inline void TCPEndpoint::m_create_socket(){
		std::memset(&this->_server_sockaddr, 0, sizeof(sockaddr_in));
		this->_server_sockaddr.sin_family = static_cast<int>(this->_domain);
		inet_pton(static_cast<int>(_domain), this->_IP_addr.c_str(), &this->_server_sockaddr.sin_addr);
		this->_server_sockaddr.sin_port = htons(this->_port);
		this->_endpoint_fd = ::socket(static_cast<int>(_domain), static_cast<int>(_sock_type), 0);
		err_check(this->_endpoint_fd, "socket() creation");
		this->_endpoint_sock_len = sizeof(this->_server_sockaddr);
	}
	inline const int TCPEndpoint::accept_loop(){
		this->_serving_client_fd = ::accept(this->_endpoint_fd, reinterpret_cast<struct sockaddr*>(&this->_client_sockaddr),
							reinterpret_cast<socklen_t*>(&this->_endpoint_sock_len));
		return this->_serving_client_fd;
	}
	inline void TCPEndpoint::bind_sock() noexcept {
		int ret_code = ::bind(this->_endpoint_fd, reinterpret_cast<sockaddr*>(&this->_server_sockaddr), sizeof(sockaddr_in));
		err_check(ret_code, "bind() error");
		this->m_listen_socket();
	}
	inline void TCPEndpoint::read_buff(char* read_buffer, std::size_t max_read_buff){
		// TODO: implementation of a BufferRead object for handling the raw bytes into iteratable object.
		// Currently in Blocking IO
		read_data(this->_serving_client_fd, read_buffer, max_read_buff, 0);
	}
	inline void TCPEndpoint::write_buff(const char* write_buffer, std::size_t write_size){
		write_data(this->_serving_client_fd, write_buffer, write_size, 0);
	}
	inline const int TCPEndpoint::get_serving_client(void) const noexcept {
		return this->_serving_client_fd;
	}
	inline const std::string& TCPEndpoint::get_ip(void) const noexcept {
		return this->_IP_addr;
	}
	inline const std::uint16_t& TCPEndpoint::get_port(void) const noexcept {
		return this->_port;
	}
	inline const int TCPEndpoint::get_endpoint_backlog(void) const noexcept {
		return this->_endpoint_backlog;
	}
	inline const Domain TCPEndpoint::get_socket_domain(void) const noexcept {
		return this->_domain;
	}
	inline const int TCPEndpoint::get_endpoint_fd(void) const noexcept {
		return this->_endpoint_fd;
	}
	inline void TCPEndpoint::close_serving_client_connection(void) noexcept {
		::close(this->_serving_client_fd);
	}
	inline std::string TCPEndpoint::get_serving_client_ip() noexcept{
		return ::inet_ntoa(this->_client_sockaddr.sin_addr);
	}
	inline void TCPEndpoint::make_socket_nonblocking(){
		int flags = ::fcntl(this->_endpoint_fd, F_GETFL, 0); 
		if(flags == -1)
		{ throw std::runtime_error("fnctl() F_GETFL"); }
		int ret = ::fcntl(this->_endpoint_fd, F_SETFL, flags | O_NONBLOCK);
		if(ret == -1)
		{ throw std::runtime_error("fnctl() O_NONBLOCK"); }
	}
	inline TCPEndpoint::~TCPEndpoint(){
		::close(this->_endpoint_fd);
		::close(this->_serving_client_fd);
	}

} // end namespace Transport
} // end namespace blueth::net
