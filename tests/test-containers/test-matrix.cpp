#include <gtest/gtest.h>
#include "matrix.hpp"

TEST(TestMatrix, Container){
	using namespace blueth;
	container::Matrix<int, 3, 6> test_matrix(
			{
			{1, 2, 3, 4, 5, 6},
			{7, 8, 9, 10, 11, 12}, 
			{14, 15, 16, 17, 18, 19}
			}
			);
	ASSERT_EQ(test_matrix.numcols(), 6);
	ASSERT_EQ(test_matrix.numrows(), 3);
	ASSERT_EQ(test_matrix[1].at(2), 9);
	std::vector<int>& col_two = test_matrix[1];
	col_two.at(1) = 1000;
	ASSERT_EQ(test_matrix[1].at(1), 1000);
}
