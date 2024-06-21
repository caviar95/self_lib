/*
 * @Author: Caviar
 * @Date: 2024-06-21 23:14:20
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-21 23:27:42
 * @Description: 冒泡排序
 * 基本思想：经过n-1轮筛选，每次将最大值放到无序区最后
 */

#include <vector>

template <typename T>
void BubbleSort(std::vector<T> &arr)
{
    auto size = arr.size();

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < size - 1 - i; ++j) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
            }
        }
    }
}
