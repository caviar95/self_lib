
#include <iostream>

#include "../include/sort/quick_sort.hpp"

void test_quick(std::vector<int> &arr)
{
    QuickSort(arr, 0, arr.size() - 1);
}

void PrintResult(const std::vector<int> &arr)
{
    for (auto val : arr) {
        std::cout << val << ' ';
    }

    std::cout << std::endl;
}

int main()
{
    std::vector<int> arr{4, 5, 2, 6, 1, 3};

    test_quick(arr);
    PrintResult(arr);

    return 0;
}