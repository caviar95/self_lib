/*
 * @Author: Caviar
 * @Date: 2024-06-22 17:13:20
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 17:50:48
 * @Description: 
 */

#include <iostream>

#include "../include/find/string_proc.hpp"


void test_max_substr_len(const std::string &str)
{
    std::cout << str << "'s max substr len: " << GetLengthOfLongestSubstring(str) << std::endl;
}

int main()
{
    std::string test{"abcabcbb"};

    test_max_substr_len(test);


    return 0;
}