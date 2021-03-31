#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
// We will not use blueth::container::vector any where, we *only* use standard
// std::vector. It's only written for fun.
namespace blueth::container {
template <typename T> class vector {
      public:
	std::size_t size() const noexcept;
	bool empty() const noexcept;

	// Adds end of list
	void push_back(const T &item) noexcept(false);
	void push_back(T &&) noexcept(false);
	// push_front is EXTREMELY inefficient, Do not use it
	void push_front(const T &item) noexcept(false);
	template <typename... Args>
	void emplace_back(Args &&... args) noexcept(false);
	// removes item end of list
	void pop_back() noexcept(false);
	// returns the object at the end of list
	const T &back() const noexcept(false);
	// returns the object at the front
	const T &front() const noexcept(false);
	void shrink_to_fit() noexcept;

	const T &operator[](std::size_t) const noexcept(false);
	T &at(std::size_t) noexcept(false);
	std::size_t capacity() const noexcept;
	explicit vector(std::size_t init_size = 0) noexcept(false);
	explicit vector(const vector &) noexcept(false);
	vector &operator=(const vector &rhs);
	vector &operator=(vector &&);
	vector(const std::initializer_list<T> &);
	explicit vector(vector &&);
	// proxy to reserve() to define complexity
	void resize(std::size_t) noexcept(false);
	~vector();
	// Iterator stuff: not bounds checked
	typedef T *iterator;
	typedef const T *const_iterator;

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	iterator end() noexcept;
	const_iterator end() const noexcept;

	typedef T value_type;
	typedef const T const_value_type;
	typedef T *pointer_type;
	typedef const T *const_pointer_type;

      private:
	std::size_t m_size{};
	std::size_t m_capacity{1};
	T *object{nullptr}; // this is the resource ultimatly managed;
	// memory allocation for new capacity;
	void m_reserve(std::size_t) noexcept(false);
};
template <typename T> inline void vector<T>::shrink_to_fit() noexcept {
	if (m_size != m_capacity) {
		std::size_t offset_to_free = m_capacity - m_size;
		for (std::size_t index{m_size}; index <= offset_to_free;
		     index++)
			std::destroy_at(object + index + 1);
		m_capacity = m_size;
	}
}
template <typename T> inline void vector<T>::push_back(const T &item) {
	if (m_capacity > m_size) {
		*(object + m_size) = item;
		m_size++;
	} else {
		m_reserve(m_capacity * 2);
		*(object + m_size) = item;
		m_size++;
	}
}
template <typename T> inline void vector<T>::push_back(T &&item) {
	if (m_capacity > m_size) {
		*(object + m_size) = std::move(item);
		m_size++;
	} else {
		m_reserve(m_capacity * 2);
		*(object + m_size) = std::move(item);
		m_size++;
	}
}
template <typename T> inline void vector<T>::push_front(const T &item) {
	if (m_capacity > m_size) {
		T *tmp_store = ::new T[m_capacity];
		*tmp_store = item;
		for (std::size_t index{1}; index < m_size; index++)
			*(tmp_store + index) = *(object + (m_size - 1));
		delete[] object;
		object = tmp_store;
		m_size++;
	} else {
		m_capacity = m_capacity * 2;
		T *tmp_store = ::new T[m_capacity];
		*tmp_store = item;
		for (std::size_t index{1}; index < m_size; index++)
			*(tmp_store + index) = *(object + (m_size - 1));
		delete[] object;
		object = tmp_store;
		m_size++;
	}
}
template <typename T>
template <typename... Args>
inline void vector<T>::emplace_back(Args &&... args) {
	if (m_capacity > m_size) {
		::new (object + ((m_size - 1) * sizeof(T)))
		    T(std::forward<Args>(args)...);
		m_size++;
	} else {
		m_reserve(m_capacity * 2);
		::new (object + ((m_size - 1) * sizeof(T)))
		    T(std::forward<Args>(args)...);
		m_size++;
	}
}
template <typename T> inline T &vector<T>::at(std::size_t index) {
	if (m_size > index) {
		return *(object + index);
	} else {
		throw std::out_of_range("at() out of bound");
	}
}
template <typename T>
inline vector<T>::vector(const std::initializer_list<T> &init_list)
    : m_size(init_list.size()), m_capacity(init_list.size() * 2) {
	object = new T[m_capacity];
	for (std::size_t index{}; index < m_size; index++)
		object[index] = std::move(*(init_list.begin() + index));
}
template <typename T> inline std::size_t vector<T>::size() const noexcept {
	return m_size;
}
template <typename T> inline bool vector<T>::empty() const noexcept {
	return m_size == 0;
}
template <typename T> inline std::size_t vector<T>::capacity() const noexcept {
	return m_capacity;
}
template <typename T>
inline const T &vector<T>::operator[](std::size_t index) const {
	if (m_size > index) {
		return *(object + index);
	} else {
		throw std::out_of_range("operator[] out of bound");
	}
}
// Copy constructor for vector<T>
template <typename T>
inline vector<T> &vector<T>::operator=(const vector<T> &rhs) {
	if (this == &rhs) { return *this; }
	vector<T> copy(rhs);
	std::swap(*this, copy);
	return *this;
}
template <typename T> inline vector<T> &vector<T>::operator=(vector<T> &&rhs) {
	if (this == &rhs) { return *this; }
	std::swap(m_capacity, rhs.m_capacity);
	std::swap(m_size, rhs.m_size);
	std::swap(object, rhs.object);
	return *this;
}
template <typename T>
inline vector<T>::vector(std::size_t init_size) : m_size(init_size) {
	object = new T[m_capacity];
}
template <typename T>
inline vector<T>::vector(const vector<T> &rhs)
    : m_capacity(rhs.m_capacity), m_size(rhs.m_size) {
	object = new T[m_capacity];
	for (std::size_t index{}; index <= m_size; index++)
		object[index] = rhs.object[index];
}
template <typename T> inline vector<T>::~vector<T>() {
	if (object) { delete[] object; }
}
template <typename T>
inline vector<T>::vector(vector &&move_rhs)
    : m_capacity(move_rhs.m_capacity), m_size(move_rhs.m_size) {
	object = move_rhs.object;
	move_rhs.object = nullptr;
	move_rhs.m_capacity = 0;
	move_rhs.m_size = 0;
}
template <typename T> inline void vector<T>::resize(std::size_t size) {
	if (size > m_capacity) {
		m_capacity = size * 2;
		m_reserve(m_capacity);
	} else {
		for (std::size_t index{size}; index < m_size; index++)
			std::destroy_at(object + index + 1);
		m_size = size;
		m_capacity = size;
	}
}
template <typename T>
inline void vector<T>::m_reserve(std::size_t allocation_size) {
	T *new_tmp = new T[allocation_size];
	for (std::size_t index{}; index < m_size; index++)
		new_tmp[index] = object[index];
	delete[] object;
	object = new_tmp;
	m_capacity = allocation_size;
}
template <typename T> inline const T &vector<T>::front() const {
	if (m_size > 0) {
		return *(object);
	} else {
		throw std::logic_error("calling fount() on empty container");
	}
}
template <typename T> inline const T &vector<T>::back() const {
	if (m_size > 0) {
		return *(object + (m_size - 1));
	} else {
		throw std::logic_error("calling back() in an empty container");
	}
}
template <typename T> inline void vector<T>::pop_back() {
	if (m_size != 0) {
		std::destroy_at(object + (m_size - 1));
		m_size--;
	} else {
		throw std::out_of_range(
		    "calling pop_back() on empty container");
	}
}
template <typename T>
inline typename vector<T>::iterator vector<T>::begin() noexcept {
	if (m_size > 0) {
		return object;
	} else {
		return nullptr;
	}
}
template <typename T>
inline typename vector<T>::iterator vector<T>::end() noexcept {
	if (m_size > 0) {
		return (object + m_size - 1);
	} else {
		return nullptr;
	}
}
template <typename T>
inline typename vector<T>::const_iterator vector<T>::begin() const noexcept {
	if (m_size > 0) {
		return object;
	} else {
		return nullptr;
	}
}
template <typename T>
inline typename vector<T>::const_iterator vector<T>::end() const noexcept {
	if (m_size > 0) {
		return (object + m_size - 1);
	} else {
		return nullptr;
	}
}
} // namespace blueth::container
