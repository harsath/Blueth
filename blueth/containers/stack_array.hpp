#pragma once
#include <alloca.h>
#include <cstring>
#include <memory>
#include <string>

namespace blueth::container {
template <typename T, std::size_t buffer_size> class StackArray {
      private:
	T *_internal_stack_buffer{nullptr};
	std::size_t _size;
	std::size_t _curr_capacity;

      public:
	explicit StackArray<T, buffer_size>();
	StackArray<T, buffer_size>(const StackArray<T, buffer_size> &) = delete;
	StackArray<T, buffer_size> &
	operator=(const StackArray<T, buffer_size> &) = delete;
	explicit StackArray<T, buffer_size>(StackArray<T, buffer_size> &&);
	operator T *() noexcept;
	std::size_t size() noexcept;
	typedef T *iterator;
	typedef const T *const const_iterator;
	iterator begin() noexcept;
	iterator end() noexcept;
	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;
	~StackArray();

      private:
	T *_stack_allocator();
};
template <typename T, std::size_t buffer_size>
inline StackArray<T, buffer_size>::StackArray() {
	_internal_stack_buffer = _stack_allocator();
}
template <typename T, std::size_t buffer_size>
inline StackArray<T, buffer_size>::StackArray(
    StackArray<T, buffer_size> &&mover_array) {
	if (&mover_array != this) {
		_size = mover_array._size;
		_curr_capacity = mover_array._curr_capacity;
		_internal_stack_buffer = mover_array._internal_stack_buffer;
		mover_array._size = 0;
		mover_array._curr_capacity = 0;
		mover_array._internal_stack_buffer = nullptr;
	}
}
template <typename T, std::size_t buffer_size>
inline StackArray<T, buffer_size>::operator T *() noexcept {
	return _internal_stack_buffer;
}
template <typename T, std::size_t buffer_size>
inline std::size_t StackArray<T, buffer_size>::size() noexcept {
	return _size;
}
template <typename T, size_t buffer_size>
inline typename StackArray<T, buffer_size>::iterator
StackArray<T, buffer_size>::begin() noexcept {
	return _internal_stack_buffer;
}
template <typename T, size_t buffer_size>
inline typename StackArray<T, buffer_size>::iterator
StackArray<T, buffer_size>::end() noexcept {
	return (_internal_stack_buffer + _size);
}
template <typename T, size_t buffer_size>
inline typename StackArray<T, buffer_size>::const_iterator
StackArray<T, buffer_size>::cbegin() const noexcept {
	return _internal_stack_buffer;
}
template <typename T, size_t buffer_size>
inline typename StackArray<T, buffer_size>::const_iterator
StackArray<T, buffer_size>::cend() const noexcept {
	return (_internal_stack_buffer + _size);
}
template <typename T, std::size_t buffer_size>
inline T *StackArray<T, buffer_size>::_stack_allocator() {
	return new (static_cast<T *>(alloca(sizeof(T) * buffer_size)))
	    T[buffer_size];
}
template <typename T, std::size_t buffer_size>
inline StackArray<T, buffer_size>::~StackArray() {
	if (_internal_stack_buffer != nullptr && _size != 0)
		std::destroy(_internal_stack_buffer,
			     _internal_stack_buffer + _size);
}

} // end namespace blueth::container
