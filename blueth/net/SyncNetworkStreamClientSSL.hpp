#pragma once
#include "NetworkStream.hpp"
#include "common.hpp"
#include "io/IOBuffer.hpp"
#include "utils/SSLHelpers.hpp"
#include "utils/UnixTime.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

namespace blueth::net {

class SyncNetworkStreamClientSSL final : public NetworkStream<char> {
      private:
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
	std::unique_ptr<WOLFSSL_CTX, detail::WolfSSL_CTX_Deleter> ctx_{nullptr};
	std::unique_ptr<WOLFSSL, detail::WolfSSL_Deleter> ssl_{nullptr};

      public:
	constexpr static std::size_t default_io_buffer_size = 2048;
	template <typename T1, typename T2, typename T3, typename T4>
	static std::unique_ptr<NetworkStream<char>>
	create(T1 &&endpoint_host, T2 &&endpoint_port, T3 &&stream_protocol,
	       T4 &&client_cert_path) {
		return std::make_unique<SyncNetworkStreamClientSSL>(
		    std::forward<T1>(endpoint_host),
		    std::forward<T2>(endpoint_port),
		    std::forward<T3>(stream_protocol),
		    std::forward<T4>(client_cert_path));
	}
	SyncNetworkStreamClientSSL(
	    std::string endpoint_host, std::uint16_t endpoint_port,
	    StreamProtocol stream_protocol,
	    std::string client_cert_path) noexcept(false);
	int streamRead(size_t read_length) noexcept(false) override;
	int streamWrite(const std::string &data) noexcept(false) override;
	void flushBuffer() noexcept(false) override;
	BLUETH_NODISCARD buffer_type getIOBuffer() noexcept(false) override;
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
	~SyncNetworkStreamClientSSL();
};

// @@@ Right now, the Sync SSL-Stream is TCP only
SyncNetworkStreamClientSSL::SyncNetworkStreamClientSSL(
    std::string endpoint_host, std::uint16_t endpoint_port,
    StreamProtocol stream_protocol,
    std::string client_cert_path) noexcept(false)
    : endpoint_host_{std::move(endpoint_host)}, endpoint_port_{endpoint_port},
      stream_protocol_{stream_protocol}, stream_mode_{StreamMode::Client},
      stream_type_{StreamType::SSLSyncStream} {

	::hostent *hoste;
	::sockaddr_in addr;
	endpoint_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
	hoste = ::gethostbyname(endpoint_host_.c_str());
	if (hoste == nullptr) {
		std::perror("gethostbyname()");
		throw std::runtime_error{std::strerror(errno)};
	}
	if (endpoint_fd_ < 0) {
		std::perror("socket()");
		throw std::runtime_error{std::strerror(errno)};
	}
	addr.sin_addr = *(reinterpret_cast<::in_addr *>(hoste->h_addr));
	// inet_pton(AF_INET, endpoint_host_.c_str(), &addr.sin_addr.s_addr);
	addr.sin_port = ::htons(endpoint_port_);
	addr.sin_family = AF_INET;
	::bzero(addr.sin_zero, 8);
	int connect_ret =
	    ::connect(endpoint_fd_, reinterpret_cast<sockaddr *>(&addr),
		      sizeof(sockaddr));
	if (connect_ret < 0) {
		std::perror("connect()");
		throw std::runtime_error{std::strerror(errno)};
	}
	int ret = wolfSSL_Init();
	if (ret != WOLFSSL_SUCCESS) {
		fprintf(stderr, "Error: Failed to init wolfSSL lib: %d\n",
			wolfSSL_get_error(ssl_.get(), ret));
		close(endpoint_fd_);
		throw std::runtime_error{"wolfSSL_Init()"};
	}
	ctx_ = std::unique_ptr<WOLFSSL_CTX, detail::WolfSSL_CTX_Deleter>(
	    wolfSSL_CTX_new(wolfTLSv1_2_client_method()));
	if (ctx_.get() == nullptr) {
		fprintf(stderr,
			"Error: failed to create WOLFSSL_CTX object: %d\n",
			wolfSSL_get_error(ssl_.get(), ret));
		close(endpoint_fd_);
		throw std::runtime_error{"wolfSSL_CTX_new()"};
	}
	ret = wolfSSL_CTX_load_verify_locations(
	    ctx_.get(), client_cert_path.c_str(), nullptr);
	if (ret != SSL_SUCCESS) {
		fprintf(stderr,
			"Error: failed to load the client cert-file: %d",
			wolfSSL_get_error(ssl_.get(), ret));
		close(endpoint_fd_);
		throw std::runtime_error{"wolfSSL_CTX_load_verify_locations"};
	}
	ssl_ = std::unique_ptr<WOLFSSL, detail::WolfSSL_Deleter>(
	    wolfSSL_new(ctx_.get()));
	if (ssl_.get() == nullptr) {
		fprintf(stderr, "Error: failed to create new WOLFSSL object\n");
		close(endpoint_fd_);
		throw std::runtime_error{"wolfSSL_new()"};
	}
	ret = wolfSSL_set_fd(ssl_.get(), endpoint_fd_);
	if (ret != SSL_SUCCESS) {
		fprintf(stderr, "Error: failed to set FD to SSL context: %d\n",
			wolfSSL_get_error(ssl_.get(), ret));
		close(endpoint_fd_);
		throw std::runtime_error{"wolfSSL_set_fd()"};
	}
	ret = wolfSSL_connect(ssl_.get());
	if (ret != SSL_SUCCESS) {
		fprintf(stderr, "Error: failed to connect to server: %d\n",
			wolfSSL_get_error(ssl_.get(), ret));
		close(endpoint_fd_);
		throw std::runtime_error{"wolfSSL_connect()"};
	}
	io_buffer_ = io::IOBuffer<char>::create(default_io_buffer_size);
	connected_time_ = UnixTime();
	is_connected_ = true;
}

int SyncNetworkStreamClientSSL::streamRead(size_t read_length) noexcept(false) {
	if (!io_buffer_ || !is_connected_) {
		throw std::runtime_error{"invalid IOBuffer or the connection "
					 "to the SSL endpoint is closed"};
	}
	if (read_length > io_buffer_->getAvailableSpace())
		io_buffer_->reserve(io_buffer_->getCapacity() + read_length);
	char temp_buffer[read_length];
	int read_ret = wolfSSL_read(ssl_.get(), temp_buffer, read_length);
	if (read_ret < 0) return read_ret;
	io_buffer_->appendRawBytes(temp_buffer, read_ret);
	if (read_callback_) read_callback_(io_buffer_);
	return read_ret;
}

/**
 * Since this is a Sync-transfer on the wire, the call gets blocked until all
 * the bytes are sent on the wire. Since it's a blocking-IO, we don't need to
 * touch the IOBuffer. So we only need to check if we actually have connection
 * open the the SSL endpoint.
 */
int SyncNetworkStreamClientSSL::streamWrite(const std::string &data) noexcept(
    false) {
	if (!is_connected_) {
		throw std::runtime_error{
		    "The connection to the SSL endpoint is closed"};
	}
	if (data.empty()) return 0;
	int write_ret = ::wolfSSL_write(ssl_.get(), data.c_str(), data.size());
	if (write_callback_) write_callback_(io_buffer_);
	return write_ret;
}

void SyncNetworkStreamClientSSL::flushBuffer() noexcept(false) {
	if (!io_buffer_) { throw std::runtime_error{"invalid IOBuffer"}; }
	io_buffer_->clear();
}

BLUETH_NODISCARD SyncNetworkStreamClientSSL::buffer_type
SyncNetworkStreamClientSSL::getIOBuffer() noexcept(false) {
	std::unique_ptr<io::IOBuffer<char>> temp_buffer_holder =
	    std::move(io_buffer_);
	io_buffer_ = nullptr;
	return std::move(temp_buffer_holder);
}

BLUETH_NODISCARD SyncNetworkStreamClientSSL::const_buffer_reference_type
SyncNetworkStreamClientSSL::constGetIOBuffer() const noexcept {
	return io_buffer_;
}

void SyncNetworkStreamClientSSL::setIOBuffer(
    SyncNetworkStreamClientSSL::buffer_type io_buffer) noexcept {
	io_buffer_ = std::move(io_buffer);
}

StreamType SyncNetworkStreamClientSSL::getStreamType() const noexcept {
	return stream_type_;
}

StreamMode SyncNetworkStreamClientSSL::getStreamMode() const noexcept {
	return stream_mode_;
}

StreamProtocol SyncNetworkStreamClientSSL::getStreamProtocol() const noexcept {
	return stream_protocol_;
}

void SyncNetworkStreamClientSSL::setReadCallback(
    std::function<void(const_buffer_reference_type)> callback_fn) noexcept {
	read_callback_ = std::move(callback_fn);
}

void SyncNetworkStreamClientSSL::setWriteCallback(
    std::function<void(const_buffer_reference_type)> callback_fn) noexcept {
	write_callback_ = std::move(callback_fn);
}

std::function<void(SyncNetworkStreamClientSSL::const_buffer_reference_type)>
SyncNetworkStreamClientSSL::getReadCallback() const noexcept {
	return read_callback_;
}

std::function<void(SyncNetworkStreamClientSSL::const_buffer_reference_type)>
SyncNetworkStreamClientSSL::getWriteCallback() const noexcept {
	return write_callback_;
}

void SyncNetworkStreamClientSSL::closeConnection() noexcept(false) {
	if (is_connected_) {
		int ret = ::close(endpoint_fd_);
		if (ret < 0) { throw std::runtime_error{std::strerror(errno)}; }
		is_connected_ = false;
	}
}

SyncNetworkStreamClientSSL::~SyncNetworkStreamClientSSL() {
	this->closeConnection();
}

} // namespace blueth::net
