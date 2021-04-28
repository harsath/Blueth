#pragma once
#include "io/IOBuffer.hpp"
#include "utils/simd.hpp"
#include <cstdint>
#include <string_view>
#include <vector>

/**
 * A codec decoder for splitting string_view frames from the input
 * of two iterators
 *
 * This is zero-copy operation since the string_views
 * only point to the Line-Frames within the two iterators rather than
 * owning their own copies(which is an expensive operation).
 * We also store the
 *
 * last used frame pointer(pointer to either CarriageReturnNewLine char or
 * NewLineNewLine char, in both cases, the pointer points to the first of the
 * two delimiters), in that way we can resume the operation from where we left
 * off last-time.
 *
 * It's very useful in non-blocking network IO where the bytes
 * come in an unpredictable/un-reliable way and we use a non-blocking IO for
 * network reads on wire.
 */
namespace blueth::codec {

enum class TerminatorType {
	CarriageReturnNewLine, /* \r\n */
	NewLineNewLine,	       /* \n\n */
};

static void
LineBasedFrameDecoder(char *begin, char *end, TerminatorType terminator_type,
		      std::vector<std::string_view> &results) noexcept {
	if (terminator_type == TerminatorType::CarriageReturnNewLine) {
		// Note: Our sse4_2 char finder will not modify the char,
		// it's casted because of the implementation detail
	FIND_IT_AGAIN_CR:
		char *cr = simd::find_char_128_sse4_2(begin, end, '\r');
		if (cr == end) return;
		if (*cr != '\r' || *(cr + 1) != '\n') return;
		size_t offset_length = (cr - begin - 1);
		results.emplace_back(std::string_view(begin, offset_length));
		// +2 to skip the '\r\n'
		begin += (offset_length + 3);
		goto FIND_IT_AGAIN_CR;
	} else { // TerminatorType::NewLineNewLine
	FIND_IT_AGAIN_LF:
		char *lf = simd::find_char_128_sse4_2(begin, end, '\n');
		if (lf == end) return;
		if ((*lf != '\n' && *(lf + 1) != '\n') || *(lf - 1) == '\r')
			return;
		size_t offset_length = (lf - begin - 1);
		results.emplace_back(std::string_view(begin, offset_length));
		// +2 to skip the '\n\n'
		begin += (offset_length + 3);
		goto FIND_IT_AGAIN_LF;
	}
}

} // namespace blueth::codec
