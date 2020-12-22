#pragma once
#include <vector>
#include <initializer_list>

namespace blueth{
namespace container{
	template<typename T, std::size_t num_row, std::size_t num_column>
	class Matrix{
		std::vector<std::vector<T>> _holder;
		public:
			Matrix(const std::initializer_list<T>& init_list)
				: _holder{init_list}{}
			Matrix(std::size_t num_coln){}
	};
}

}
