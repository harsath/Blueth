#include <gtest/gtest.h>
#include "vector.hpp"

TEST(TestVector, ContainerTest){
	using namespace blueth;
	container::vector<int> test_vector = {1, 2, 3, 4, 5, 6};

	ASSERT_EQ(test_vector.size(), 6);

	ASSERT_EQ(test_vector.capacity(), 12);

	ASSERT_EQ(test_vector[2], 3);

	EXPECT_ANY_THROW(test_vector[100]);

	ASSERT_EQ(test_vector.at(5), 6);

	EXPECT_ANY_THROW(test_vector.at(6));
	test_vector.push_back(7);

	ASSERT_EQ(test_vector.at(6), 7);
	ASSERT_EQ(test_vector.capacity(), 12);

	test_vector.shrink_to_fit();
	EXPECT_TRUE(
		(test_vector.capacity() == test_vector.size()) && (test_vector.size() == 7)
		);

	test_vector.pop_back();
	EXPECT_TRUE(test_vector.size() == 6);
	EXPECT_TRUE(test_vector.back() == 6);
	EXPECT_TRUE(test_vector.capacity() == 7);
	EXPECT_TRUE(test_vector.front() == 1);

	test_vector.resize(3);
	EXPECT_TRUE(test_vector.size() == 3);
	ASSERT_EQ(test_vector.capacity(), 3);

	test_vector.resize(10);
	EXPECT_TRUE(test_vector.size() == 3);
	ASSERT_EQ(test_vector.capacity(), 20);

	{
		container::vector<int> foo;
		EXPECT_ANY_THROW(foo.front());
		EXPECT_ANY_THROW(foo.pop_back());
		EXPECT_ANY_THROW(foo.back());
		foo.push_back(100);
		EXPECT_NO_THROW(foo.front());
		EXPECT_NO_THROW(foo.back());
	}

	{
		container::vector<int> foo;
		foo.push_back(1);
		foo.push_back(2);
		foo.push_back(3);
		foo.push_back(5);
		foo.push_back(6);
		foo.push_back(7);
		foo.push_back(8);
		foo.push_back(9);
		foo.push_back(10);
		ASSERT_EQ(foo.front(), 1);
		ASSERT_EQ(foo.back(), 10);
		ASSERT_EQ(foo.size(), 9);
		ASSERT_EQ(foo.capacity(), 16);
		ASSERT_EQ(foo.begin(), &foo.at(0));
		ASSERT_EQ(foo.end(), &foo.at(foo.size()-1));

		foo.push_back(11);
		ASSERT_EQ(foo.front(), 1);
		ASSERT_EQ(foo.back(), 11);
		ASSERT_EQ(foo.size(), 10);
		ASSERT_EQ(foo.capacity(), 16);
		ASSERT_EQ(foo.begin(), &foo.at(0));
		ASSERT_EQ(foo.end(), &foo.at(foo.size()-1));

		foo.pop_back();
		ASSERT_EQ(foo.front(), 1);
		ASSERT_EQ(foo.back(), 10);
		ASSERT_EQ(foo.size(), 9);
		ASSERT_EQ(foo.capacity(), 16);
		ASSERT_EQ(foo.begin(), &foo.at(0));
		ASSERT_EQ(foo.end(), &foo.at(foo.size()-1));

		foo.resize(24);
		ASSERT_EQ(foo.front(), 1);
		ASSERT_EQ(foo.back(), 10);
		ASSERT_EQ(foo.size(), 9);
		ASSERT_EQ(foo.capacity(), 48);
		ASSERT_EQ(foo.begin(), &foo.at(0));
		ASSERT_EQ(foo.end(), &foo.at(foo.size()-1));

		foo.shrink_to_fit();
		ASSERT_EQ(foo.front(), 1);
		ASSERT_EQ(foo.back(), 10);
		ASSERT_EQ(foo.size(), 9);
		ASSERT_EQ(foo.capacity(), 9);
		ASSERT_EQ(foo.begin(), &foo.at(0));
		ASSERT_EQ(foo.end(), &foo.at(foo.size()-1));
	}
}
