#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

/* IOBuffer is a small abstraction over raw uint8_t bytes internally used for Async state recorder & handlers
 * It manages a uint8_t/char bytes and keeps track of used offsets within the memory
 * Primarily used for Async implementation which operates over Non-Blocking network socket IO
 *
 *        start_offset             end_offset
 *           |                       |
 *           v                       v
 *    +------+---------------------+------------+          (buffer_end - buffer_start) = capacity of underlying memory
 *    | [1]  |        Data         |   Un-Init  |           [1] => Already *sent* bytes through Non-blocking socket(headroom)
 *    +------+---------------------+------------+
 *    ^                                         ^
 *    |                                         |
 *    buffer_start                            buffer_end
 *
 *         end_offset points to memory one location past the last element(Data memory region)
 *     
 *          start_offset    end_offset
 *	       |           |
 *	       v           v
 *	     +---+---+---+---+
 *	     | H | e | y | ~ |          	           ~ => Un-initialized memory
 *	     +---+---+---+---+
 */

namespace blueth::io {

template<typename T> struct IOBufTraits;
template<> struct IOBufTraits<char> {
	using value_type = char;
	using reference_type = char&;
	using const_reference_type = const char&;
	using pointer_type = char*;
	using const_pointer_type = const char*;
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
	using const_pointer_type = const std::uint8_t*;
	using iterator = std::uint8_t*;
	using const_iterator = const std::uint8_t*;
	using size_type = std::size_t;
	using difference_type = int;
};
template<typename T> struct is_byte_type : std::false_type {};
template<> struct is_byte_type<char> : std::true_type {};
template<> struct is_byte_type<std::uint8_t> : std::true_type {};

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> = true>
class IOBuffer {
	public:
	  using value_type 		= typename IOBufTraits<T>::value_type;
	  using reference_type 		= typename IOBufTraits<T>::reference_type;
	  using const_reference_type 	= typename IOBufTraits<T>::const_reference_type;
	  using pointer_type 		= typename IOBufTraits<T>::pointer_type;
	  using const_pointer_type	= typename IOBufTraits<T>::const_pointer_type;
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
	  ~IOBuffer();
	  constexpr IOBuffer(IOBuffer&& io_buffer) noexcept ;
	  constexpr IOBuffer& operator=(IOBuffer&& io_buffer) noexcept ;
	  IOBuffer(const IOBuffer&) = delete;
	  IOBuffer& operator=(const IOBuffer&) = delete;
	  /**
	   * Update the start offset of the underlying buffer
	   *
	   * @param offset_len Amount of bytes sent through the Non-blocking sockets. May pass signe/unsigned value
	   */
	  constexpr void setStartOffset(int offset_len) noexcept;
	  constexpr void modifyStartOffset(int offset_len) noexcept;
	  constexpr size_type getStartOffset() const noexcept;
	  /**
	   * Update the end offset of the underlying buffer
	   *
	   * @param offset_len Length to add/sub the end data offset
	   */
	  constexpr void setEndOffset(int offset_len) noexcept;
	  constexpr void modifyEndOffset(int offset_len) noexcept;
	  constexpr size_type getEndOffset() const noexcept;
	  /**
	   * Get an arbitary offset of bytes from the actual operational data of the buffer
	   *
	   * @param start_offset Offset to start from
	   * @param end_offset offset to end
	   * @return <Begin Pointer, End Pointer>
	   */
	  constexpr std::pair<const_pointer_type, const_pointer_type> 
		  getOffset(size_type start_offset, size_type end_offset) const noexcept;
	  /**
	   * Get offset from start_offset to offset_len and return const_pointer_type
	   *
	   * @param offset_len End offset of bytes(from start_offset_)
	   */
	  constexpr std::pair<const_pointer_type, const_pointer_type> 
		  getOffset(size_type offset_len) const noexcept;
	  /**
	   * Append raw bytes to end of data offset on the buffer
	   *
	   * @param bytes Raw data to be appended
	   * @param size Size of bytes to be appended
	   */
	  constexpr void appendRawBytes(const_pointer_type data, size_type size) noexcept(false);
	  /**
	   * Append data bytes form other IOBuffer into current buffer
	   *
	   * @param io_buffer Source IOBuffer object
	   */
	  constexpr void appendRawBytes(const IOBuffer<value_type>& io_buffer) noexcept(false);
	  /**
	   * Clear the buffer
	   */
	  constexpr void clear() noexcept;
	  /**
	   * Return the pointer to start underlying buffer
	   */
	  constexpr const_pointer_type getBuffer() const noexcept;
	  /**
	   * Return the pointer to end of underlying buffer
	   */
	  constexpr const_pointer_type getBufferEnd() const noexcept;
	  /**
	   * Get a pointer to start offset of actual working data(internal_buffer_(start_offset_))
	   */
	  constexpr const_pointer_type getStartOffsetPointer() const noexcept;
	  /**
	   * Get a pointer to end offset of actual working data(internal_buffer_(end_offset_))
	   */
	  constexpr const_pointer_type getEndOffsetPointer() const noexcept;
	  /**
	   * Get current internal capacity of underlying buffer
	   */
	  constexpr size_type getCapacity() const noexcept;
	  /**
	   * Get amount of memory/space left before we need ran our and allocate (capacity - end_offset)
	   */
	  constexpr size_type getAvailableSpace() const noexcept;
	  /**
	   * Get actual working data/buffer's size (end_offset - start_offset)
	   */
	  constexpr size_type getDataSize() const noexcept;

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
	  void reserve(size_type mem_size = 0) noexcept(false);
	private:
	  pointer_type internal_buffer_{nullptr};
	  size_type start_offset_{};
	  size_type end_offset_{};
	  size_type capacity_{};
};

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline std::unique_ptr<IOBuffer<T>> IOBuffer<T, U>::create(typename IOBufTraits<T>::size_type capacity){
	return std::make_unique<IOBuffer<T>>(capacity);
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline IOBuffer<T, U>::IOBuffer(typename IOBufTraits<T>::size_type capacity){
	reserve(capacity);
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr IOBuffer<T, U>::IOBuffer(IOBuffer<T, U>&& io_buffer) noexcept {
	std::swap(internal_buffer_, io_buffer.internal_buffer_);
	std::swap(start_offset_,io_buffer.start_offset_);
	std::swap(end_offset_, io_buffer.end_offset_);
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr IOBuffer<T, U>& IOBuffer<T, U>::operator=(IOBuffer<T, U>&& io_buffer) noexcept {
	if(!internal_buffer_){
		std::swap(internal_buffer_, io_buffer.internal_buffer_);
		std::swap(start_offset_,io_buffer.start_offset_);
		std::swap(end_offset_, io_buffer.end_offset_);
	}
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline void IOBuffer<T, U>::reserve(typename IOBufTraits<T>::size_type mem_size) noexcept(false) {
	if(!mem_size){ mem_size = capacity_ * 2; }
	typename IOBufTraits<T>::pointer_type tmp_ptr = (typename IOBufTraits<T>::pointer_type)
								::realloc(internal_buffer_, mem_size);	
	if(tmp_ptr == nullptr){ 
		throw std::bad_alloc{};
	}
	internal_buffer_ = tmp_ptr;
	capacity_ = mem_size;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::size_type IOBuffer<T, U>::getStartOffset() const noexcept {
	return start_offset_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void IOBuffer<T, U>::clear() noexcept {
	// Next time we do any write operation, we "overwrite" the previous bytes
	start_offset_ = 0;
	end_offset_ = 0;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::size_type IOBuffer<T, U>::getEndOffset() const noexcept {
	return end_offset_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void IOBuffer<T, U>::modifyStartOffset(int offset_len) noexcept {
	start_offset_ += offset_len;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void IOBuffer<T, U>::setStartOffset(int offset_len) noexcept {
	start_offset_ = offset_len;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::size_type IOBuffer<T, U>::getDataSize() const noexcept {
	return end_offset_ - start_offset_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline IOBuffer<T, U>::~IOBuffer<T, U>(){
	if(internal_buffer_ != nullptr){ ::free(internal_buffer_); }
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::size_type IOBuffer<T, U>::getCapacity() const noexcept {
	return capacity_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::const_pointer_type IOBuffer<T, U>::getBuffer() const noexcept {
	return internal_buffer_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::const_pointer_type IOBuffer<T, U>::getStartOffsetPointer() const noexcept {
	return internal_buffer_ + start_offset_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::const_pointer_type IOBuffer<T, U>::getEndOffsetPointer() const noexcept {
	return internal_buffer_ + end_offset_;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::const_pointer_type IOBuffer<T, U>::getBufferEnd() const noexcept {
	return (internal_buffer_ + capacity_-1);
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void IOBuffer<T, U>::setEndOffset(int offset_len) noexcept {
	end_offset_ = offset_len;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void IOBuffer<T, U>::modifyEndOffset(int offset_len) noexcept {
	end_offset_ += offset_len;
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::iterator IOBuffer<T, U>::begin() noexcept {
	return &internal_buffer_[start_offset_];
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::iterator IOBuffer<T, U>::end() noexcept {
	return &internal_buffer_[end_offset_];
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::const_iterator IOBuffer<T, U>::cbegin() const noexcept {
	return &internal_buffer_[start_offset_];
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::const_iterator IOBuffer<T, U>::cend() const noexcept {
	return &internal_buffer_[end_offset_];
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr std::pair<typename IOBufTraits<T>::const_pointer_type, typename IOBufTraits<T>::const_pointer_type> 
IOBuffer<T, U>::getOffset(
		typename IOBufTraits<T>::size_type start_offset, typename IOBufTraits<T>::size_type end_offset) const noexcept {
	if(start_offset <= capacity_ && end_offset <= capacity_){
		return {&internal_buffer_[start_offset], &internal_buffer_[end_offset]};
	}
	return {nullptr, nullptr};
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr std::pair<typename IOBufTraits<T>::const_pointer_type, typename IOBufTraits<T>::const_pointer_type> 
IOBuffer<T, U>::getOffset(
		typename IOBufTraits<T>::size_type offset_len) const noexcept {
	if(offset_len <= capacity_){
		return {&internal_buffer_[start_offset_], &internal_buffer_[offset_len]};
	}
	return {nullptr, nullptr};
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr typename IOBufTraits<T>::size_type IOBuffer<T, U>::getAvailableSpace() const noexcept {
	return capacity_ - (end_offset_);
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void 
IOBuffer<T, U>::appendRawBytes(typename IOBufTraits<T>::const_pointer_type data, typename IOBufTraits<T>::size_type size) noexcept(false) {
	if(getAvailableSpace() >= size){
		::memcpy(internal_buffer_ + end_offset_, data, size);
		end_offset_ += size;
	}else{
		reserve(size);
		appendRawBytes(data, size);
	}
}

template<typename T, std::enable_if_t<is_byte_type<T>::value, bool> U>
inline constexpr void
IOBuffer<T, U>::appendRawBytes(const IOBuffer<typename IOBufTraits<T>::value_type>& io_buffer) noexcept(false) {
	if(io_buffer.getDataSize() <= getAvailableSpace()){
		::memcpy(internal_buffer_ + end_offset_, io_buffer.getStartOffsetPointer(), io_buffer.getDataSize());
		end_offset_ += io_buffer.getDataSize();
	}else{
		reserve(io_buffer.getDataSize());
		appendRawBytes(io_buffer);
	}
}

} // end namespace blueth::io
