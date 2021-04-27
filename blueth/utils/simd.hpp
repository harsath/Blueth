#pragma once
#include "common.hpp"
#include <ctime>
#include <emmintrin.h>
#include <immintrin.h>
#include <iostream>
#include <nmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>

namespace blueth::simd {
/**
 * This class 'bool_vector_128_sse2' is only used as the container for storing
 * vector of either 0xFF or 0x00, and the intrinsic function which returns the
 * __m128i vector as a result of some operation is stored here.
 */
class bool_vector_128 {
      private:
	__m128i data_;

      public:
	BLUETH_FORCE_INLINE explicit bool_vector_128(__m128i data) noexcept
	    : data_{data} {}
	BLUETH_FORCE_INLINE friend bool_vector_128
	operator&(const bool_vector_128 &x, const bool_vector_128 &y) noexcept {
		return bool_vector_128(_mm_and_si128(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_128
	operator|(const bool_vector_128 &x, const bool_vector_128 &y) noexcept {
		return bool_vector_128(_mm_or_si128(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE int mask() const noexcept {
		return _mm_movemask_epi8(data_);
	}
	BLUETH_FORCE_INLINE int
	get_first_true_in_mask(int mask) const noexcept {
		return __builtin_ctz(mask);
	}
	BLUETH_FORCE_INLINE int get_first_true_in_mask() const noexcept {
		return __builtin_ctz(_mm_movemask_epi8(data_));
	}
};

class char_vector_128 {
      private:
	__m128i data_;

      public:
	static constexpr int size = 16;
	static constexpr int bit_size = 128;
	BLUETH_FORCE_INLINE explicit char_vector_128(__m128i data) noexcept
	    : data_{data} {}
	BLUETH_FORCE_INLINE friend bool_vector_128
	operator==(char_vector_128 &x, char_vector_128 &y) noexcept {
		return bool_vector_128(_mm_cmpeq_epi8(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_128
	operator<(char_vector_128 x, char_vector_128 y) noexcept {
		return bool_vector_128(_mm_cmplt_epi8(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_128
	operator>(char_vector_128 x, char_vector_128 y) noexcept {
		return bool_vector_128(_mm_cmpgt_epi8(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_128
	operator|(char_vector_128 x, char_vector_128 y) noexcept {
		return bool_vector_128(_mm_or_si128(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE __m128i m128i() const noexcept { return data_; }
};

class bool_vector_256 {
      private:
	__m256i data_;

      public:
	BLUETH_FORCE_INLINE explicit bool_vector_256(__m256i data) noexcept
	    : data_{data} {}
	BLUETH_FORCE_INLINE friend bool_vector_256
	operator&(const bool_vector_256 &x, const bool_vector_256 &y) noexcept {
		return bool_vector_256(_mm256_and_si256(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_256
	operator|(const bool_vector_256 &x, const bool_vector_256 &y) noexcept {
		return bool_vector_256(_mm256_or_si256(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE int mask() const noexcept {
		return _mm256_movemask_epi8(data_);
	}
	BLUETH_FORCE_INLINE int
	get_first_true_in_mask(int mask) const noexcept {
		return __builtin_ctz(mask);
	}
	BLUETH_FORCE_INLINE int get_first_true_in_mask() const noexcept {
		return __builtin_ctz(_mm256_movemask_epi8(data_));
	}
};

class char_vector_256 {
      private:
	__m256i data_;

      public:
	static constexpr int size = 32;
	static constexpr int bit_size = 256;
	BLUETH_FORCE_INLINE explicit char_vector_256(__m256i data) noexcept
	    : data_{data} {}
	BLUETH_FORCE_INLINE friend bool_vector_256
	operator==(char_vector_256 x, char_vector_256 y) noexcept {
		return bool_vector_256(_mm256_cmpeq_epi8(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_256
	operator<(char_vector_256 x, char_vector_256 y) noexcept {
		return bool_vector_256(_mm256_cmpeq_epi8(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE friend bool_vector_256
	operator|(char_vector_256 x, char_vector_256 y) noexcept {
		return bool_vector_256(_mm256_or_si256(x.data_, y.data_));
	}
	BLUETH_FORCE_INLINE __m256i m256i() const noexcept { return data_; }
};

static char *find_char_256_avx2(char *buffer_start, char *buffer_end,
				char find_char) {
	char_vector_256 find_char_vector{_mm256_set1_epi8(find_char)};
	for (; buffer_start + 32 <= buffer_end; buffer_start += 32) {
		char_vector_256 buffer_pack{
		    _mm256_lddqu_si256((__m256i *)buffer_start)};
		char_vector_256 result_cmp{_mm256_cmpeq_epi8(
		    find_char_vector.m256i(), buffer_pack.m256i())};
		bool_vector_256 buffer_eq_find_char =
		    (buffer_pack == find_char_vector);
		if (buffer_eq_find_char.mask())
			return buffer_start +
			       buffer_eq_find_char.get_first_true_in_mask();
	}
	for (; buffer_start < buffer_end; ++buffer_start)
		if (*buffer_start == find_char) return buffer_start;
	return buffer_end;
}

static char *find_char_128_sse4_2(char *buffer_start, char *buffer_end,
				  char find_char) {
	char_vector_128 find_char_vector{_mm_set1_epi8(find_char)};
	for (; buffer_start + 16 < buffer_end; buffer_start += 16) {
		char_vector_128 buffer_pack{
		    _mm_lddqu_si128((__m128i *)buffer_start)};
		char_vector_128 result_cmp{_mm_cmpeq_epi8(
		    find_char_vector.m128i(), buffer_pack.m128i())};
		bool_vector_128 buffer_eq_find_char =
		    (buffer_pack == find_char_vector);
		if (buffer_eq_find_char.mask())
			return buffer_start +
			       buffer_eq_find_char.get_first_true_in_mask();
	}
	for (; buffer_start < buffer_end; ++buffer_start)
		if (*buffer_start == find_char) return buffer_start;
	return buffer_end;
}

} // end namespace blueth::simd
