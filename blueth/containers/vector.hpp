#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <utility>
namespace blueth{
namespace container{
	template<typename T> class vector{
		public:
			std::size_t size() const noexcept;
			bool empty() const noexcept;

			// Adds end of list
			void push_back(const T& item);
			void push_back(T&&);
			template<typename... Args> 
			void emplace_back(Args&&... args);
			// removes item end of list
			void pop_back();
			// returns the object at the end of list
			const T& back() const;
			// returns the object at the front
			const T& front() const;
			void shrink_to_fit() noexcept;

			const T& operator[](std::size_t) const;
			T& at(std::size_t);
			std::size_t capacity() const noexcept;
			explicit vector(std::size_t init_size = 0);
			explicit vector(const vector&);
			vector& operator=(const vector& rhs);
			vector& operator=(vector&&);
			vector(const std::initializer_list<T>&);
			explicit vector(vector&&);
			// proxy to this->reserve() to define complexity
			void resize(std::size_t);
			~vector();
			// Iterator stuff: not bounds checked
			typedef T* iterator;
			typedef const T* const_iterator;

			iterator begin() noexcept;
			const_iterator begin() const noexcept;
			iterator end() noexcept;
			const_iterator end() const noexcept;

			typedef T value_type;
			typedef const T const_value_type;
			typedef T* pointer_type;
			typedef const T* const_pointer_type;
		private:
			std::size_t m_size{};
			std::size_t m_capacity{1};
			T* object{nullptr}; // this is the resource ultimatly managed;
			// memory allocation for new capacity;
			void m_reserve(std::size_t);
	};
	template<typename T> inline void vector<T>::shrink_to_fit() noexcept {
		if(this->m_size != this->m_capacity){
			std::size_t offset_to_free = this->m_capacity - this->m_size;		
			for(std::size_t index{this->m_size}; index <= offset_to_free; index++)
				std::destroy_at(object + index+1);
			this->m_capacity = this->m_size;
		}
	}
	template<typename T> inline void vector<T>::push_back(const T& item){
		if(this->m_capacity > this->m_size){
			*(object + this->m_size) = item;
			this->m_size++;
		}else{
			try{
				this->m_reserve(this->m_capacity * 2);
				*(object + this->m_size) = item;
				this->m_size++;
			}catch(std::exception exceptions){
				throw exceptions.what();
			}
		}
	}
	template<typename T> inline void vector<T>::push_back(T&& item){
		if(this->m_capacity > this->m_size){
			*(object + this->m_size) = std::move(item);
			this->m_size++;
		}else{
			try{
				this->m_reserve(this->m_capacity * 2);
				*(object + this->m_size) = std::move(item);
				this->m_size++;
			}catch(const std::exception& exceptions){
				throw exceptions.what();
			}
		}
	}
	template<typename T>
	template<typename... Args>
	inline void vector<T>::emplace_back(Args&&... args){
		if(this->m_capacity > this->m_size){
			::new(this->object + ((this->m_size-1)*sizeof(T))) T(std::forward<Args>(args)...);
			this->m_size++;
		}else{
			try{
				this->m_reserve(this->m_capacity * 2);
				::new(this->object + ((this->m_size-1)*sizeof(T))) T(std::forward<Args>(args)...);
				this->m_size++;
			}catch(const std::exception& exceptions){
				throw exceptions.what();
			}
		}
	}
	template<typename T> inline T& vector<T>::at(std::size_t index){
		if(this->m_size > index){
			return *(object + index);
		}else{
			throw "operator[] out of bound";
		}
	}
	template<typename T> inline vector<T>::vector(const std::initializer_list<T>& init_list)
		: m_size(init_list.size()), m_capacity(init_list.size()*2){
		this->object = new T[m_capacity];
		for(std::size_t index{}; index < this->m_size; index++)
			object[index] = std::move(*(init_list.begin()+index));
	}
	template<typename T> inline std::size_t vector<T>::size() const noexcept {
		return this->m_size;
	}
	template<typename T> inline bool vector<T>::empty() const noexcept {
		return this->m_size == 0;
	}
	template<typename T> inline std::size_t vector<T>::capacity() const noexcept {
		return this->m_capacity;
	}
	template<typename T> inline const T& vector<T>::operator[](std::size_t index) const {
		if(this->m_size > index){
			return *(object + index);
		}else{
			throw "operator[] out of bound";
		}
	}
	// Copy constructor for vector<T>
	template<typename T> inline vector<T>& vector<T>::operator=(const vector<T> &rhs){
		if(this == &rhs){ return *this; }		
		vector<T> copy(rhs);
		std::swap(*this, copy);
		return *this;
	}
	template<typename T> inline vector<T>& vector<T>::operator=(vector<T>&& rhs){
		if(this == &rhs){ return *this; }		
		std::swap(this->m_capacity, rhs.m_capacity);
		std::swap(this->m_size, rhs.m_size);
		std::swap(this->object, rhs.object);
		return *this;
	}
	template<typename T> inline vector<T>::vector(std::size_t init_size)
		: m_size(init_size){
			this->object = new T[m_capacity];
	}
	template<typename T> inline vector<T>::vector(const vector<T>& rhs)
		: m_capacity(rhs.m_capacity), m_size(rhs.m_size){
			this->object = new T[m_capacity];
			for(std::size_t index{}; index <= m_size; index++)
				this->object[index] = rhs.object[index];
	}
	template<typename T> inline vector<T>::~vector<T>(){
		delete[] this->object;
	}
	template<typename T> inline vector<T>::vector(vector&& move_rhs)
		: m_capacity(move_rhs.m_capacity), m_size(move_rhs.m_size){
		this->object = move_rhs.object;
		move_rhs.object = nullptr;
		move_rhs.m_capacity = 0;
		move_rhs.m_size = 0;
	}
	template<typename T> inline void vector<T>::resize(std::size_t size){
		if(size > this->m_capacity){
			this->m_capacity = size * 2;
			this->m_reserve(this->m_capacity);
		}else{
			for(std::size_t index{size}; index < this->m_size; index++)
				std::destroy_at((object + index+1));
			this->m_size = size;
			this->m_capacity = size;
		}
	}
	template<typename T> inline void vector<T>::m_reserve(std::size_t allocation_size){
		T* new_tmp = new(std::nothrow) T[allocation_size];	
		if(new_tmp != nullptr){
			for(std::size_t index{}; index < this->m_size; index++)
				new_tmp[index] = this->object[index];
			delete[] this->object;
			this->object = new_tmp;
			this->m_capacity = allocation_size;
		}else{
			throw "Memory error, but items preserved";
		}
	}
	template<typename T> inline const T& vector<T>::front() const {
		if(this->m_size > 0){
			return *(object);
		}else{
			throw "calling fount() on empty container";
		}
	}
	template<typename T> inline const T& vector<T>::back() const {
		if(this->m_size > 0){
			return *(object + (m_size-1));
		}else{
			throw "calling back() in an empty container";
		}
	}
	template<typename T> inline void vector<T>::pop_back(){
		if(this->m_size != 0){
			std::destroy_at(object + (this->m_size-1));
			this->m_size--;
		}else{
			throw "calling pop_back() on empty container";
		}
	}
	template<typename T> inline typename vector<T>::iterator vector<T>::begin() noexcept {
		if(this->m_size > 0){ return object; }
		else{ return nullptr; }
	}
	template<typename T> inline typename vector<T>::iterator vector<T>::end() noexcept {
		if(this->m_size > 0){ return (object + m_size-1); }
		else{ return nullptr; }
	}
	template<typename T> inline typename vector<T>::const_iterator vector<T>::begin() const noexcept {
		if(this->m_size > 0){ return object; }
		else{ return nullptr; }
	}
	template<typename T> inline typename vector<T>::const_iterator vector<T>::end() const noexcept {
		if(this->m_size > 0){ return (object + m_size-1); }
		else{ return nullptr; }
	}
}
}
