#pragma once
#include "NetworkStream.hpp"
#include "common.hpp"
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
#include <unistd.h>

namespace blueth::net {

class SyncNetworkStreamClient final : public NetworkStream<char> {
	std::string endpoint_host_;
	std::uint16_t endpoint_port_;
	int endpoint_fd_;
	StreamProtocol stream_protocol_;
	StreamType stream_type_;
	StreamMode stream_mode_;
	UnixTime connected_time_;
	std::function<void(const_buffer_reference_type)> read_callback_{
	    nullptr};
	std::function<void(const_buffer_reference_type)> write_callback_{
	    nullptr};
	std::unique_ptr<io::IOBuffer<char>> io_buffer_{nullptr};
	bool is_connected_{false};

      public:
	constexpr static std::size_t default_io_buffer_size = 2048;
	SyncNetworkStreamClient(std::string endpoint_ip,
				std::uint16_t endpoint_port,
				StreamProtocol stream_protocol) noexcept(false);
	int streamRead(size_t read_length) noexcept(false) override;
	int streamWrite() noexcept(false) override;
	BLUETH_FORCE_INLINE void flushBuffer() noexcept(false) override;
	BLUETH_FORCE_INLINE BLUETH_NODISCARD buffer_type
	getIOBuffer() noexcept(false) override;
	BLUETH_FORCE_INLINE BLUETH_NODISCARD const_buffer_reference_type
	constGetIOBuffer() const noexcept(false) override;
	BLUETH_FORCE_INLINE void
	setIOBuffer(buffer_type io_buffer) noexcept override;
	BLUETH_FORCE_INLINE StreamType getStreamType() const noexcept override;
	BLUETH_FORCE_INLINE StreamMode getStreamMode() const noexcept override;
	BLUETH_FORCE_INLINE StreamProtocol
	getStreamProtocol() const noexcept override;
	BLUETH_FORCE_INLINE void setReadCallback(
	    std::function<void(const_buffer_reference_type)>) noexcept override;
	BLUETH_FORCE_INLINE void setWriteCallback(
	    std::function<void(const_buffer_reference_type)>) noexcept override;
	BLUETH_FORCE_INLINE std::function<void(const_buffer_reference_type)>
	getReadCallback() const noexcept override;
	BLUETH_FORCE_INLINE std::function<void(const_buffer_reference_type)>
	getWriteCallback() const noexcept override;
	BLUETH_FORCE_INLINE void closeConnection() noexcept(false) override;
	~SyncNetworkStreamClient();
};

SyncNetworkStreamClient::SyncNetworkStreamClient(
    std::string endpoint_host, std::uint16_t endpoint_port,
    StreamProtocol stream_protocol) noexcept(false)
    : endpoint_host_{std::move(endpoint_host)}, endpoint_port_{endpoint_port},
      stream_protocol_{stream_protocol}, stream_mode_{StreamMode::Client},
      stream_type_{StreamType::SyncStream} {
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
	io_buffer_ = io::IOBuffer<char>::create(default_io_buffer_size);
	connected_time_ = UnixTime();
	is_connected_ = true;
}

int SyncNetworkStreamClient::streamRead(size_t read_length) noexcept(false) {
	if (!io_buffer_ || !is_connected_) {
		throw std::runtime_error{"invalid IOBuffer or the connection "
					 "to the endpoint is closed"};
	}
	if (read_length > io_buffer_->getAvailableSpace())
		io_buffer_->reserve(io_buffer_->getCapacity() + read_length);
	int read_ret = ::recv(endpoint_fd_, io_buffer_->getStartOffsetPointer(),
			      io_buffer_->getAvailableSpace(), 0);
	if (read_ret < 0) return read_ret;
	io_buffer_->modifyStartOffset(read_ret);
	if (read_callback_) read_callback_(io_buffer_);
	return read_ret;
}

int SyncNetworkStreamClient::streamWrite() noexcept(false) {
	if (!io_buffer_ || !is_connected_) {
		throw std::runtime_error{"invalid IOBuffer or the connection "
					 "to the endpoint is closed"};
	}
	/// We have nothing to write onto the endpoint, so we return 0
	/// indicating we didn't write anything on the wire
	if (io_buffer_->getDataSize() == 0) { return 0; }
	int write_ret =
	    ::send(endpoint_fd_, io_buffer_->getStartOffsetPointer(),
		   io_buffer_->getDataSize(), 0);
	if (write_ret < 0) return write_ret;
	io_buffer_->modifyEndOffset(write_ret);
	if (write_callback_) write_callback_(io_buffer_);
	return write_ret;
}

BLUETH_FORCE_INLINE void
SyncNetworkStreamClient::flushBuffer() noexcept(false) {
	if (!io_buffer_) { throw std::runtime_error{"invalid IOBuffer"}; }
	io_buffer_->clear();
}

BLUETH_NODISCARD BLUETH_NODISCARD SyncNetworkStreamClient::buffer_type
SyncNetworkStreamClient::getIOBuffer() noexcept(false) {
	std::unique_ptr<io::IOBuffer<char>> temp_buffer_holder =
	    std::move(io_buffer_);
	io_buffer_ = nullptr;
	return std::move(temp_buffer_holder);
}

BLUETH_FORCE_INLINE
BLUETH_NODISCARD SyncNetworkStreamClient::const_buffer_reference_type
SyncNetworkStreamClient::constGetIOBuffer() const noexcept(false) {
	return io_buffer_;
}

BLUETH_FORCE_INLINE void SyncNetworkStreamClient::setIOBuffer(
    SyncNetworkStreamClient::buffer_type io_buffer) noexcept {
	io_buffer_ = std::move(io_buffer);
}

BLUETH_FORCE_INLINE StreamType
SyncNetworkStreamClient::getStreamType() const noexcept {
	return stream_type_;
}

BLUETH_FORCE_INLINE StreamMode
SyncNetworkStreamClient::getStreamMode() const noexcept {
	return stream_mode_;
}

BLUETH_FORCE_INLINE StreamProtocol
SyncNetworkStreamClient::getStreamProtocol() const noexcept {
	return stream_protocol_;
}

BLUETH_FORCE_INLINE void SyncNetworkStreamClient::setReadCallback(
    std::function<void(SyncNetworkStreamClient::const_buffer_reference_type)>
	callback_fn) noexcept {
	read_callback_ = std::move(callback_fn);
}

BLUETH_FORCE_INLINE void SyncNetworkStreamClient::setWriteCallback(
    std::function<void(const_buffer_reference_type)> callback_fn) noexcept {
	write_callback_ = std::move(callback_fn);
}

BLUETH_FORCE_INLINE
std::function<void(SyncNetworkStreamClient::const_buffer_reference_type)>
SyncNetworkStreamClient::getReadCallback() const noexcept {
	return read_callback_;
}

BLUETH_FORCE_INLINE
std::function<void(SyncNetworkStreamClient::const_buffer_reference_type)>
SyncNetworkStreamClient::getWriteCallback() const noexcept {
	return write_callback_;
}

BLUETH_FORCE_INLINE void
SyncNetworkStreamClient::closeConnection() noexcept(false) {
	if (is_connected_) {
		int ret = ::close(endpoint_fd_);
		if (ret < 0) { throw std::runtime_error{std::strerror(errno)}; }
		is_connected_ = false;
	}
}

SyncNetworkStreamClient::~SyncNetworkStreamClient() { this->closeConnection(); }

} // namespace blueth::net
