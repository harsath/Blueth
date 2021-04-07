#pragma once
#include "NetworkStream.hpp"
#include "io/IOBuffer.hpp"
#include "utils/UnixTime.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>

namespace blueth::net {

class SyncNetworkStreamClient final : public NetworkStream<char> {
	std::string endpoint_host_;
	std::uint16_t endpoint_port_;
	int endpoint_fd_;
	StreamProtocol stream_protocol_;
	UnixTime connected_time_;
	std::function<void(const_buffer_reference_type)> read_callback_;
	std::function<void(const_buffer_reference_type)> write_callback_;

      public:
	SyncNetworkStreamClient(std::string endpoint_ip,
				std::uint16_t endpoint_port,
				StreamProtocol stream_protocol) noexcept(false);
	int streamRead(size_t read_length) noexcept(false) override;
	int streamWrite() noexcept(false) override;
	void flushBuffer() noexcept(false) override;
	buffer_type getIOBuffer() noexcept(false) override;
	const_buffer_reference_type constGetIOBuffer() const
	    noexcept(false) override;
	void setIOBuffer(buffer_type io_buffer) noexcept override;
	StreamType getStreamType() const noexcept override;
	StreamMode getStreamMode() const noexcept override;
	StreamProtocol getStreamProtocol() const noexcept override;
	void setReadCallback(std::function<void(const_buffer_reference_type)>) noexcept override;
	void setWriteCallback(std::function<void(const_buffer_reference_type)>) noexcept override;
	std::function<void(const_buffer_reference_type)> getReadCallback() const noexcept override;
	std::function<void(const_buffer_reference_type)> getWriteCallback() const noexcept override;
};

SyncNetworkStreamClient::SyncNetworkStreamClient(
    std::string endpoint_host, std::uint16_t endpoint_port,
    StreamProtocol stream_protocol) noexcept(false)
    : endpoint_host_{std::move(endpoint_host)}, endpoint_port_{endpoint_port},
      stream_protocol_{stream_protocol} {
	::addrinfo hints, *server_info, *temp;
	::hostent *hoste;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	if (stream_protocol == StreamProtocol::TCP) {
		hints.ai_socktype = SOCK_STREAM;
	} else if (stream_protocol == StreamProtocol::UDP) {
		hints.ai_socktype = SOCK_DGRAM;
	}
	int return_val;
	if ((return_val = ::getaddrinfo(endpoint_host_.c_str(), nullptr, &hints,
					&server_info))) {
		std::perror("getaddrinfo");
		throw std::runtime_error{std::strerror(errno)};
	}
	temp = server_info;
	for (; temp != nullptr; temp = temp->ai_next) {
		if ((endpoint_fd_ = ::socket(temp->ai_family, temp->ai_socktype,
					     temp->ai_protocol)) == -1) {
			continue;
		}
		if (::connect(endpoint_fd_, temp->ai_addr, temp->ai_addrlen) ==
		    -1) {
			continue;
		}
		break;
	}
	if (temp == nullptr) {
		std::perror("socket/connect");
		throw std::runtime_error{std::strerror(errno)};
	}
	::freeaddrinfo(server_info);
}

} // namespace blueth::net
