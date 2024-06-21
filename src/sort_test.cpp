/*
 * @Author: Caviar
 * @Date: 2024-06-21 22:56:49
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 01:37:09
 * @Description: 
 */

#include <iostream>

#include "../include/sort/bubble_sort.hpp"
#include "../include/sort/quick_sort.hpp"
#include "../include/sort/select_sort.hpp"
#include "../include/sort/insert_sort.hpp"
#include "../include/sort/shell_sort.hpp"

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

void test_insert(std::vector<int> arr)
{
    InsertSort(arr);
    PrintResult("InsertSort", arr);
}

void test_shell(std::vector<int> arr)
{
    ShellSort(arr);
    PrintResult("ShellSort", arr);
}

int main()
{
    std::vector<int> arr{7, 4, 5, 2, 6, 1, 3, 3};

    test_quick(arr);
    test_bubble(arr);
    test_select(arr);
    test_insert(arr);
    test_shell(arr);

    return 0;
}