/*
 * @Author: Caviar
 * @Date: 2024-06-21 22:56:49
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-21 23:43:02
 * @Description: 
 */

#include <iostream>

#include "../include/sort/bubble_sort.hpp"
#include "../include/sort/quick_sort.hpp"
#include "../include/sort/select_sort.hpp"

void PrintResult(const std::string sortStr, const std::vector<int> &arr)
{
    std::cout << sortStr << std::endl;

    for (auto val : arr) {
        std::cout << val << ' ';
    }

    std::cout << std::endl;
}

void test_quick(std::vector<int> arr)
{
    QuickSort(arr);
    PrintResult("QuickSort", arr);
}

void test_bubble(std::vector<int> arr)
{
    BubbleSort(arr);
    PrintResult("BubbleSort", arr);
}

void test_select(std::vector<int> arr)
{
    SelectSort(arr);
    PrintResult("SelectSort", arr);
}

int main()
{
    std::vector<int> arr{7, 4, 5, 2, 6, 1, 3, 3};

    test_quick(arr);
    test_bubble(arr);
    test_select(arr);

    return 0;
}