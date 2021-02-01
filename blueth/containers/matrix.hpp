#pragma once
#include <vector>
#include <initializer_list>

namespace blueth{
namespace container{
	template<typename T, std::size_t num_row, std::size_t num_column>
	class Matrix{
		std::vector<std::vector<T>> _holder;
		public:
			explicit Matrix(const std::vector<std::vector<T>>&);	
			explicit Matrix(std::vector<std::vector<T>>&&);	
			const std::vector<T>& operator[](std::size_t row) const noexcept;
			std::vector<T>& operator[](std::size_t row) noexcept;
			std::size_t numrows() const noexcept;
			std::size_t numcols() const noexcept;
	};
	template<typename T, std::size_t num_row, std::size_t num_column>
	inline std::size_t Matrix<T, num_row, num_column>::numcols() const noexcept {
		return num_column;
	}
	template<typename T, std::size_t num_row, std::size_t num_column>
	inline std::size_t Matrix<T, num_row, num_column>::numrows() const noexcept {
		return num_row;
	}
	template<typename T, std::size_t num_row, std::size_t num_column>
	inline const std::vector<T>& Matrix<T, num_row, num_column>::operator[](std::size_t row) const noexcept {
		return _holder.at(row);
	}
	template<typename T, std::size_t num_row, std::size_t num_column>
	inline std::vector<T>& Matrix<T, num_row, num_column>::operator[](std::size_t row) noexcept {
		return _holder.at(row);
	}
	template<typename T, std::size_t num_row, std::size_t num_column>
		inline Matrix<T, num_row, num_column>::Matrix(
				const std::vector<std::vector<T>>& holder) 
			: _holder(holder){
		}
	template<typename T, std::size_t num_row, std::size_t num_column>
		inline Matrix<T, num_row, num_column>::Matrix(
				std::vector<std::vector<T>>&& holder) 
			: _holder(std::move(holder)){
		}
}

}
