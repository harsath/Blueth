#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>

/* IOBuffer is a small abstraction over raw uint8_t bytes internally used for Async state recorder & handlers
 * It manages a uint8_t bytes and keepts track of used offsets within the memory
 * Primarily used for Async implementation which operates over Non-Blocking network socket IO
 *
 *        start_offset           end_offset
 *           |                     |
 *           v                     v
 *    +------+---------------------+------------+          (buffer_end - buffer_start) = capacity of underlying memory
 *    | [1]  |        Data         |   Un-Init  |           [1] => Already *sent* bytes through Non-blocking socket
 *    +------+---------------------+------------+
 *    ^                                         ^
 *    |                                         |
 *    buffer_start                            buffer_end
 */

namespace io {

template<typename T> struct IOBufTraits;
template<> struct IOBufTraits<char> {
	using value_type = char;
	using reference_type = char&;
	using const_reference_type = const char&;
	using pointer_type = char*;
	using iterator = char*;
	using const_iterator = const char*;
	using size_type = std::size_t;
	using difference_type = int;
};
template<> struct IOBufTraits<std::uint8_t> {
	using value_type = std::uint8_t;
	using reference_type = std::uint8_t&;
	using const_reference_type = const std::uint8_t&;
	using pointer_type = std::uint8_t*;
	using iterator = std::uint8_t*;
	using const_iterator = const std::uint8_t*;
	using size_type = std::size_t;
	using difference_type = int;
};
template<typename T> struct is_byte_type : std::false_type {};
template<> struct is_byte_type<char> : std::true_type {};
template<> struct is_byte_type<std::uint8_t> : std::true_type {};

template<typename T=std::uint8_t, std::enable_if_t<is_byte_type<T>::value, bool> = true>
class IOBuffer {
	public:
	  using value_type 		= typename IOBufTraits<T>::value_type;
	  using reference_type 		= typename IOBufTraits<T>::reference_type;
	  using const_reference_type 	= typename IOBufTraits<T>::const_reference_type;
	  using pointer_type 		= typename IOBufTraits<T>::pointer_type;
	  using iterator 		= typename IOBufTraits<T>::iterator;
	  using const_iterator 		= typename IOBufTraits<T>::const_iterator;
	  using size_type 		= typename IOBufTraits<T>::size_type;
	  using difference_type 	= typename IOBufTraits<T>::difference_type;
	  /**
	   * Allocate a new IOBuffer object with requested capacity
	   *
	   * @param capacity Initial capacity of the underlying buffer
	   * @return Unique pointer to IOBuffer<T> object
	   */
	  static std::unique_ptr<IOBuffer<T>> create(size_type capacity);
	  IOBuffer(size_type capacity);
	  IOBuffer(IOBuffer&& io_buffer);
	  IOBuffer& operator=(IOBuffer&& io_buffer);
	  IOBuffer(const IOBuffer&) = delete;
	  IOBuffer& operator=(const IOBuffer&) = delete;
	  /**
	   * Update the start offset of the underlying buffer
	   *
	   * @param offset_len Amount of bytes sent through the Non-blocking sockets. May pass signe/unsigned value
	   */
	  constexpr void setStartOffset(int offset_len) noexcept;
	  constexpr size_type getStartOffset() const noexcept;
	  /**
	   * Update the end offset of the underlying buffer
	   *
	   * @param offset_len Length to add/sub the end data offset
	   */
	  constexpr void setEndOffset(int offset_len) noexcept;
	  constexpr size_type getEndOffset() const noexcept;
	  /**
	   * Get a particular offset of bytes from the actual operational data of the buffer
	   *
	   * @param start_offset Offset to start from
	   * @param end_offset offset to end
	   */
	  constexpr pointer_type getOffset(size_type start_offset, size_type end_offset) const noexcept;
	  /**
	   * Append raw bytes to end of data offset on the buffer
	   *
	   * @param bytes Raw data to be appended
	   * @param size Size of bytes to be appended
	   */
	  constexpr void appendRawBytes(pointer_type data, size_type size);
	  /**
	   * Append data bytes form other IOBuffer into current buffer
	   *
	   * @param io_buffer Source IOBuffer object
	   */
	  constexpr void appendRawBytes(const IOBufTraits<std::uint8_t>& io_buffer) noexcept;
	  /**
	   * Clear the buffer
	   */
	  constexpr void clear() noexcept;
	  /**
	   * Free/Deallocate the underlying buffer memory
	   */
	  void free() noexcept;
	  /**
	   * Return the pointer to start underlying buffer
	   */
	  constexpr pointer_type getBuffer() const noexcept;
	  /**
	   * Return the pointer to end of underlying buffer
	   */
	  constexpr pointer_type getBufferEnd() const noexcept;
	  /**
	   * Get a pointer to start offset of actual working data(internal_buffer_(start_offset_))
	   */
	  constexpr pointer_type getStartOffsetPointer() const noexcept;
	  /**
	   * Get a pointer to end offset of actual working data(internal_buffer_(end_offset_))
	   */
	  constexpr pointer_type getEndOffsetPointer() const noexcept;
	  /**
	   * Get current internal capacity of underlying buffer
	   */
	  constexpr size_type getCapacity() const noexcept;
	  /**
	   * Get amount of headroom(start of internal_buffer to currently used data)
	   */
	  constexpr size_type headroom() const noexcept;
	  /**
	   * Get amount of memory/space left before we need ran our and allocate
	   */
	  constexpr size_type getAvailableSpace() const noexcept;
	  /**
	   * Get actual working data/buffer's size (end_offset - start_offset)
	   */
	  constexpr size_type getDataSize() const noexcept;
	  /**
	   * Convenience function to destroy/deallocate a buffer(calls the Dtor)
	   */
	  static void destroy(std::unique_ptr<IOBufTraits<T>> io_buffer);
	  constexpr iterator begin() noexcept;
	  constexpr iterator end() noexcept;
	  constexpr const_iterator cbegin() const noexcept;
	  constexpr const_iterator cend() const noexcept;
	private:
	  /**
	   * Allocate the memory to the underlying buffer
	   *
	   * @param mem_size Num bytes to be allocated
	   * Internally implemented as `capacity = 2*capacity;`
	   */
	  void reserve(size_type mem_size) noexcept(false);
	private:
	  pointer_type internal_buffer_;
	  size_type start_offset_;
	  size_type end_offset_;
};

// TODO: implementation
template<>
inline std::unique_ptr<IOBuffer<>> IOBuffer<>::create(size_type capacity){}

} // end namespace io
