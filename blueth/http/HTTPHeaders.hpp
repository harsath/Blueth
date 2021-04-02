#pragma once
#include "HTTPConstants.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include "common.hpp"

namespace blueth::http {
// A thin wrapper around std::unordered_map container for managing HTTP headers
// easily
class HTTPHeaders {
      private:
	std::unordered_map<std::string, std::string> http_headers_;

      public:
	HTTPHeaders() {}
	void
	addHeader(std::pair<std::string, std::string> header_pair) noexcept;
	bool removeHeader(const std::string &header_name) noexcept;
	std::string buildRawHeader() const noexcept;
	std::optional<std::string>
	getHeaderValue(const std::string &header_name) const noexcept;
	std::size_t headerCount() const noexcept;
	bool headerContains(const std::string &http_name) const noexcept;
	~HTTPHeaders() {}
};

inline void HTTPHeaders::addHeader(
    std::pair<std::string, std::string> header_pair) noexcept {
	http_headers_.emplace(std::move(header_pair));
}

inline bool HTTPHeaders::removeHeader(const std::string &header_name) noexcept {
	if (!http_headers_.contains(header_name)) return false;
	http_headers_.erase(header_name);
	return true;
}

BLUETH_FORCE_INLINE inline bool
HTTPHeaders::headerContains(const std::string &header_name) const noexcept {
	return http_headers_.contains(header_name);
}

BLUETH_FORCE_INLINE inline std::size_t HTTPHeaders::headerCount() const noexcept {
	return http_headers_.size();
}

inline std::optional<std::string>
HTTPHeaders::getHeaderValue(const std::string &header_name) const noexcept {
	if (!http_headers_.contains(header_name)) return std::nullopt;
	return http_headers_.at(header_name);
}

inline std::string HTTPHeaders::buildRawHeader() const noexcept {
	std::string returner;
	for (const std::pair<std::string, std::string> &header :
	     http_headers_) {
		returner += header.first;
		returner += ": ";
		returner += header.second;
		returner += "\r\n";
	}
	return (returner += "\r\n");
}

} // namespace blueth::http
