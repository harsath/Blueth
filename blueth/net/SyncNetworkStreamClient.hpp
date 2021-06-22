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
#include <netinet/in.h>
#include <stdexcept>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

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
	template <typename T1, typename T2, typename T3>
	static std::unique_ptr<NetworkStream<char>>
	create(T1 &&endpoint_host, T2 &&endpoint_port, T3 &&stream_protocol) {
		return std::make_unique<SyncNetworkStreamClient>(
		    std::forward<T1>(endpoint_host),
		    std::forward<T2>(endpoint_port),
		    std::forward<T3>(stream_protocol));
	}
	SyncNetworkStreamClient(std::string endpoint_host,
				std::uint16_t endpoint_port,
				StreamProtocol stream_protocol) noexcept(false);
	int streamRead(size_t read_length) noexcept(false) override;
	int streamWrite(const std::string &data) noexcept(false) override;
	void flushBuffer() noexcept(false) override;
	BLUETH_NODISCARD buffer_type getIOBuffer() noexcept override;
	BLUETH_NODISCARD const_buffer_reference_type
	constGetIOBuffer() const noexcept override;
	void setIOBuffer(buffer_type io_buffer) noexcept override;
	StreamType getStreamType() const noexcept override;
	StreamMode getStreamMode() const noexcept override;
	StreamProtocol getStreamProtocol() const noexcept override;
	void setReadCallback(
	    std::function<void(const_buffer_reference_type)>) noexcept override;
	void setWriteCallback(
	    std::function<void(const_buffer_reference_type)>) noexcept override;
	std::function<void(const_buffer_reference_type)>
	getReadCallback() const noexcept override;
	std::function<void(const_buffer_reference_type)>
	getWriteCallback() const noexcept override;
	void closeConnection() noexcept(false) override;
	~SyncNetworkStreamClient();
};

SyncNetworkStreamClient::SyncNetworkStreamClient(
    std::string endpoint_host, std::uint16_t endpoint_port,
    StreamProtocol stream_protocol) noexcept(false)
    : endpoint_host_{std::move(endpoint_host)}, endpoint_port_{endpoint_port},
      stream_protocol_{stream_protocol}, stream_mode_{StreamMode::Client},
      stream_type_{StreamType::SyncStream} {
	::hostent *hoste;
	::sockaddr_in addr;
	// @@@ gethostbyname alternative
	if ((hoste = ::gethostbyname(endpoint_host_.c_str())) == nullptr) {
		throw std::runtime_error{std::strerror(errno)};
	}
	if (stream_protocol_ == StreamProtocol::TCP) {
		endpoint_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
	} else if (stream_protocol_ == StreamProtocol::UDP) {
		endpoint_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
	} else {
		throw std::runtime_error{"invalid StreamProtocol"};
	}
	if (endpoint_fd_ < 0) {
		std::perror("socket()");
		throw std::runtime_error{std::strerror(errno)};
	}
	addr.sin_addr = *(reinterpret_cast<::in_addr *>(hoste->h_addr));
	addr.sin_port = ::htons(endpoint_port_);
	addr.sin_family = AF_INET;
	::bzero(addr.sin_zero, 8);
	int connect_ret =
	    ::connect(endpoint_fd_, reinterpret_cast<sockaddr *>(&addr),
		      sizeof(addr));
	if (connect_ret < 0) {
		std::perror("connect()");
		throw std::runtime_error{std::strerror(errno)};
	}
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
	char temp_buffer[read_length];
	int read_ret = ::recv(endpoint_fd_, temp_buffer, read_length, 0);
	if (read_ret < 0) return read_ret;
	io_buffer_->appendRawBytes(temp_buffer, read_ret);
	if (read_callback_) read_callback_(io_buffer_);
	return read_ret;
}

/**
 * Since it's a blocking-IO byte transfer on the wire, we don't need to touch
 * the underlying IOBuffer so we only need to check if we transfered the bytes
 * directly from 'data' onto the wire. Using a IOBuffer is a level of
 * indirection, which we don't want since this interface is Sync and not Async
 * network-IO.
 */

int SyncNetworkStreamClient::streamWrite(const std::string &data) noexcept(
    false) {
	if (!is_connected_) {
		throw std::runtime_error{
		    "The TCP connection to the endpoint is not connected"};
	}
	if (data.empty()) return 0;
	int write_ret = ::send(endpoint_fd_, data.c_str(), data.size(), 0);
	if (write_callback_) write_callback_(io_buffer_);
	return write_ret;
}

void SyncNetworkStreamClient::flushBuffer() noexcept(false) {
	if (!io_buffer_) { throw std::runtime_error{"invalid IOBuffer"}; }
	io_buffer_->clear();
}

BLUETH_NODISCARD SyncNetworkStreamClient::buffer_type
SyncNetworkStreamClient::getIOBuffer() noexcept {
	std::unique_ptr<io::IOBuffer<char>> temp_buffer_holder =
	    std::move(io_buffer_);
	io_buffer_ = nullptr;
	return std::move(temp_buffer_holder);
}

BLUETH_NODISCARD SyncNetworkStreamClient::const_buffer_reference_type
SyncNetworkStreamClient::constGetIOBuffer() const noexcept {
	return io_buffer_;
}

void SyncNetworkStreamClient::setIOBuffer(
    SyncNetworkStreamClient::buffer_type io_buffer) noexcept {
	io_buffer_ = std::move(io_buffer);
}

StreamType SyncNetworkStreamClient::getStreamType() const noexcept {
	return stream_type_;
}

StreamMode SyncNetworkStreamClient::getStreamMode() const noexcept {
	return stream_mode_;
}

StreamProtocol SyncNetworkStreamClient::getStreamProtocol() const noexcept {
	return stream_protocol_;
}

void SyncNetworkStreamClient::setReadCallback(
    std::function<void(SyncNetworkStreamClient::const_buffer_reference_type)>
	callback_fn) noexcept {
	read_callback_ = std::move(callback_fn);
}

void SyncNetworkStreamClient::setWriteCallback(
    std::function<void(const_buffer_reference_type)> callback_fn) noexcept {
	write_callback_ = std::move(callback_fn);
}

std::function<void(SyncNetworkStreamClient::const_buffer_reference_type)>
SyncNetworkStreamClient::getReadCallback() const noexcept {
	return read_callback_;
}

std::function<void(SyncNetworkStreamClient::const_buffer_reference_type)>
SyncNetworkStreamClient::getWriteCallback() const noexcept {
	return write_callback_;
}

void SyncNetworkStreamClient::closeConnection() noexcept(false) {
	if (is_connected_) {
		int ret = ::close(endpoint_fd_);
		if (ret < 0) { throw std::runtime_error{std::strerror(errno)}; }
		is_connected_ = false;
	}
}

SyncNetworkStreamClient::~SyncNetworkStreamClient() { this->closeConnection(); }

} // namespace blueth::net
