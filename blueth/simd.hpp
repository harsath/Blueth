#pragma once
#include <emmintrin.h>
#include <iostream>

#if defined(__clang__) || defined(__GNUC__)
#define BLUETH_FORCE_INLINE [[gnu::always_inline]]
#elif
#define BLUETH_FORCE_INLINE
#endif

namespace blueth::simd {
	/* This class 'bool_vector_16_sse2' is only used as the container for storing vector of either 0xFF or 0x00,
	 * and the intrinsic function which returns the __m128i vector as a result of some operation is stored here.
	 */
	class bool_vector_16_sse2{
		private:
		  __m128i data_;
		public:
		  BLUETH_FORCE_INLINE explicit bool_vector_16_sse2(__m128i data) noexcept
			  : data_{data} {}
		  BLUETH_FORCE_INLINE friend bool_vector_16_sse2 operator&(
				const bool_vector_16_sse2& x, const bool_vector_16_sse2& y) noexcept {
			return bool_vector_16_sse2(_mm_and_si128(x.data_, y.data_));
		  }
		  BLUETH_FORCE_INLINE friend bool_vector_16_sse2 operator|(
				const bool_vector_16_sse2& x, const bool_vector_16_sse2& y) noexcept {
			return bool_vector_16_sse2(_mm_or_si128(x.data_, y.data_));
		  }
		  BLUETH_FORCE_INLINE uint32_t mask() const noexcept {
			  return static_cast<uint32_t>(_mm_movemask_epi8(data_));
		  }
	};
	class char_vector_16_sse2{
		private:
		  __m128i data_;
		public:
		  static constexpr int size = 16;
		  BLUETH_FORCE_INLINE explicit char_vector_16_sse2(__m128i data) noexcept
			  : data_{data} {}
		  BLUETH_FORCE_INLINE friend bool_vector_16_sse2 operator==(char_vector_16_sse2 x, char_vector_16_sse2 y) noexcept {
			  return bool_vector_16_sse2(_mm_cmpeq_epi8(x.data_, y.data_));
		  }
		  BLUETH_FORCE_INLINE friend bool_vector_16_sse2 operator<(char_vector_16_sse2 x, char_vector_16_sse2 y) noexcept {
			  return bool_vector_16_sse2(_mm_cmplt_epi8(x.data_, y.data_));
		  }
		  BLUETH_FORCE_INLINE friend bool_vector_16_sse2 operator>(char_vector_16_sse2 x, char_vector_16_sse2 y) noexcept {
			  return bool_vector_16_sse2(_mm_cmpgt_epi8(x.data_, y.data_));
		  }
		  BLUETH_FORCE_INLINE friend bool_vector_16_sse2 operator|(char_vector_16_sse2 x, char_vector_16_sse2 y) noexcept {
			  return bool_vector_16_sse2(_mm_or_si128(x.data_, y.data_));
		  }
		  BLUETH_FORCE_INLINE __m128i m128i() const noexcept { return data_; }
	};

} // end namespace blueth::simd
