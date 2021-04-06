#pragma once
#include "common.hpp"
#include <string>
#include <time.h>

namespace blueth {

class UnixTime {
      public:
	UnixTime() noexcept;
	UnixTime(time_t unix_time) noexcept;
	UnixTime(tm time_struct) noexcept;
	BLUETH_FORCE_INLINE std::string getStringTime() const noexcept;
	BLUETH_FORCE_INLINE time_t getUnixTime() const noexcept;
	BLUETH_FORCE_INLINE void setUnixTime(time_t unix_time) noexcept;
	BLUETH_FORCE_INLINE void setUnixTimeStruct(tm time_struct) noexcept;
	BLUETH_FORCE_INLINE int getSeconds() const noexcept;
	BLUETH_FORCE_INLINE int getMinutes() const noexcept;
	BLUETH_FORCE_INLINE int getHours() const noexcept;
	BLUETH_FORCE_INLINE int getDayOfTheMonth() const noexcept;
	BLUETH_FORCE_INLINE int getMonth() const noexcept;
	BLUETH_FORCE_INLINE int getDayOfTheWeek() const noexcept;
	BLUETH_FORCE_INLINE int getYear() const noexcept;
	BLUETH_FORCE_INLINE static time_t getNow() noexcept;
	BLUETH_FORCE_INLINE static std::string
	getStringFromUnixTime(time_t unix_time) noexcept;
	BLUETH_FORCE_INLINE constexpr friend bool
	operator==(const UnixTime &x, const UnixTime &y) noexcept {
		return x.unix_time_ == y.unix_time_;
	}
	BLUETH_FORCE_INLINE constexpr friend bool
	operator!=(const UnixTime &x, const UnixTime &y) noexcept {
		return x.unix_time_ != y.unix_time_;
	}
	BLUETH_FORCE_INLINE constexpr friend bool
	operator<(const UnixTime &x, const UnixTime &y) noexcept {
		return x.unix_time_ < y.unix_time_;
	}
	BLUETH_FORCE_INLINE constexpr friend bool
	operator>(const UnixTime &x, const UnixTime &y) noexcept {
		return x.unix_time_ > y.unix_time_;
	}
	BLUETH_FORCE_INLINE friend UnixTime
	operator-(const UnixTime &x, const UnixTime &y) noexcept {
		return UnixTime{x.unix_time_ - y.unix_time_};
	}
	BLUETH_FORCE_INLINE friend UnixTime operator+(const UnixTime &x,
						      const UnixTime &y) {
		return UnixTime{x.unix_time_ + y.unix_time_};
	}

      private:
	time_t unix_time_;
	tm time_struct_;
};

UnixTime::UnixTime() noexcept {
	::time(&unix_time_);
	time_struct_ = *::localtime(&unix_time_);
}

UnixTime::UnixTime(time_t unix_time) noexcept
    : unix_time_{unix_time}, time_struct_{*::localtime(&unix_time)} {}

UnixTime::UnixTime(tm time_struct) noexcept
    : unix_time_{::mktime(&time_struct)}, time_struct_{time_struct} {}

BLUETH_FORCE_INLINE inline std::string
UnixTime::getStringTime() const noexcept {
	char buffer[100];
	::strftime(buffer, sizeof(buffer), "%a %Y-%m-%d %H:%M:%S %Z",
		   &time_struct_);
	return buffer;
}

BLUETH_FORCE_INLINE inline time_t UnixTime::getUnixTime() const noexcept {
	return unix_time_;
}

BLUETH_FORCE_INLINE inline void
UnixTime::setUnixTime(time_t unix_time) noexcept {
	unix_time_ = unix_time;
	time_struct_ = *::localtime(&unix_time);
}

BLUETH_FORCE_INLINE inline void
UnixTime::setUnixTimeStruct(tm time_struct) noexcept {
	time_struct_ = time_struct;
	unix_time_ = ::mktime(&time_struct);
}

BLUETH_FORCE_INLINE inline int UnixTime::getDayOfTheMonth() const noexcept {
	return time_struct_.tm_mday;
}

BLUETH_FORCE_INLINE inline int UnixTime::getDayOfTheWeek() const noexcept {
	return time_struct_.tm_wday;
}

BLUETH_FORCE_INLINE inline int UnixTime::getHours() const noexcept {
	return time_struct_.tm_hour;
}

BLUETH_FORCE_INLINE inline int UnixTime::getMinutes() const noexcept {
	return time_struct_.tm_min;
}

BLUETH_FORCE_INLINE inline int UnixTime::getMonth() const noexcept {
	return time_struct_.tm_mon;
}

BLUETH_FORCE_INLINE inline int UnixTime::getSeconds() const noexcept {
	return time_struct_.tm_sec;
}

BLUETH_FORCE_INLINE inline int UnixTime::getYear() const noexcept {
	return time_struct_.tm_year;
}

time_t UnixTime::getNow() noexcept {
	time_t returner;
	return ::time(&returner);
}

BLUETH_FORCE_INLINE std::string
UnixTime::getStringFromUnixTime(time_t unix_time) noexcept {
	char buffer[100];
	tm time_struct = *::localtime(&unix_time);
	::strftime(buffer, sizeof(buffer), "%a %Y-%m-%d %H:%M:%S %Z",
		   &time_struct);
	return buffer;
}

} // namespace blueth
