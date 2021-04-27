#pragma once
#include "io/IOBuffer.hpp"
#include "utils/simd.hpp"
#include <cstdint>
#include <string_view>
#include <vector>

/**
 * A codec decoder for splitting string_view frames from the input
 * io::IOBuffer<char> buffer. This is zero-copy operation since the string_views
 * only point to the Line-Frames within the io::IOBuffer<char> rather than
 * owning their own copies(which is an expensive operation). We also store the
 * last used frame pointer, in that way we can resume the operation from where
 * we left off last-time. It's very useful in non-blocking network IO where the
 * bytes come in an unpredictable/un-reliable way and we use a non-blocking IO
 * for network reads on wire.
 */
namespace blueth::codec {

class LineBasedFrameDecoder {
	std::uint32_t max_frames_;
	std::uint32_t current_frame_index_{};
	const std::unique_ptr<io::IOBuffer<char>> &io_buffer;
	io::IOBuffer<char>::const_pointer_type last_processed_new_line_{
	    nullptr};

      public:
	enum class TerminatorType {
		CarriageReturnNewLine, /* \r\n */
		NewLineNewLine,	       /* \n\n */
	};

      public:
	LineBasedFrameDecoder(
	    const std::unique_ptr<io::IOBuffer<char>> &io_buffer,
	    std::uint32_t max_frames = UINT32_MAX) noexcept
	    : io_buffer{io_buffer}, max_frames_{max_frames} {}
};

} // namespace blueth::codec
