#pragma once
#include <iostream>

namespace blueth{
namespace container{
	template<typename T> class list{
		private:
			struct Node{
				T data;
				Node* next_node;
				Node* prev_node;
				Node(const T& data_, 
					Node* next_ptr = nullptr, 
					Node* prev_ptr = nullptr) 
					: data(data_), next_node(next_ptr), prev_node(prev_ptr) {}
				Node(T&& data_, 
					Node* next_ptr = nullptr, 
					Node* prev_ptr = nullptr)
					: data(std::move(data_)), next_node(std::move(next_ptr)), prev_node(std::move(prev_ptr)) {}
			};
		public:
			class const_iterator{
				protected:
					list<T>::Node* m_current_node;
						
				public:
					const_iterator() : m_current_node{nullptr} {}
					// return the object stored in current position
					// i.e: constant reference
					const T& operator*() const { return m_get_value(); }
					// post-inc
					const_iterator operator++(int){
						const_iterator returner = *this;
						++(*this);
						return returner;
					}
					// pre-inc
					const_iterator& operator++(){
						this->m_current_node = this->m_current_node->next_ptr;
						return *this;
					}
					// post-dec
					const_iterator operator--(int){
						const_iterator returner = *this;
						--(*this);
						return returner;
					}
					// pre-dec
					const_iterator& operator--(){
						this->m_current_node = m_current_node->prev_node;
						return *this;
					}
					bool operator==(const const_iterator& rhs) const {
						return m_current_node==rhs.m_current_node;
					}
					bool operator!=(const const_iterator& rhs) const {
						return m_current_node != rhs.m_current_node;
					}
				protected:
					T& m_get_value(){
						return this->m_current_node->data;
					}
					const_iterator(list<T>::Node* ptr) : m_current_node{ptr} {}
					friend class list<T>;
			}; // end class const_iterator
			class iterator : public const_iterator {
				public:
					iterator() {}
					T& operator*(){
						return const_iterator::m_get_value();
					}
					iterator& operator++(){
						this->m_current_node = this->m_current_node->next_ptr;
						return *this;
					}
					iterator operator++(int){
						iterator returner = this->m_current_node;
						this->m_current_node = this->m_current_node->next_ptr;
						return returner;
					}
					iterator& operator--(){
						this->m_current_node = this->m_current_node->prev_node;
						return *this;
					}
					iterator operator--(int){
						iterator returner = this->m_current_node;	
						this->m_current_node = this->m_current_node->prev_node;
						return returner;
					}
				protected:
					iterator(list<T>::Node* ptr) : const_iterator{ptr} {}
					friend class list<T>;
			};
			list(){ init(); }

	};
}
}
