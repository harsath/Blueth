#pragma once
#include "http_constants.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include "common.hpp"

namespace blueth::http {
// A thin wrapper around std::unordered_map container for managing HTTP headers
// easily
class http_headers {
      private:
	std::unordered_map<std::string, std::string> http_headers_;

      public:
	http_headers() {}
	void
	add_header(std::pair<std::string, std::string> header_pair) noexcept;
	bool remove_header(const std::string &header_name) noexcept;
	std::string build_raw_header() const noexcept;
	std::optional<std::string>
	get_header_value(const std::string &header_name) const noexcept;
	std::size_t header_count() const noexcept;
	bool header_contains(const std::string &http_name) const noexcept;
	~http_headers() {}
};

void http_headers::add_header(
    std::pair<std::string, std::string> header_pair) noexcept {
	http_headers_.emplace(std::move(header_pair));
}

bool http_headers::remove_header(const std::string &header_name) noexcept {
	if (!http_headers_.contains(header_name)) return false;
	http_headers_.erase(header_name);
	return true;
}

BLUETH_FORCE_INLINE bool
http_headers::header_contains(const std::string &header_name) const noexcept {
	return http_headers_.contains(header_name);
}

BLUETH_FORCE_INLINE std::size_t http_headers::header_count() const noexcept {
	return http_headers_.size();
}

std::optional<std::string>
http_headers::get_header_value(const std::string &header_name) const noexcept {
	if (!http_headers_.contains(header_name)) return std::nullopt;
	return http_headers_.at(header_name);
}

std::string http_headers::build_raw_header() const noexcept {
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
