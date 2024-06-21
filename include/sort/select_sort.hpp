/*
 * @Author: Caviar
 * @Date: 2024-06-21 23:29:04
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 01:33:13
 * @Description: 选择排序
 * 基本思想：
 *  1 从第一个元素开始，找无序区最小/大的元素，最后与区域第一个值交换，此时，区域边界值划分到有序区
 *  2 依次循环，直至全部变为有序区
 * 
 * 缺点：时间复杂度永远为O(n^2)
 */

#pragma once

#include <vector>

template <typename T>
void SelectSort(std::vector<T> &arr)
{
    auto size = arr.size();

    for (auto i = 0; i < size; ++i) {
        size_t maxPos = 0;
        for (int j = 0; j < size - i; ++j) {
            if (arr[j] > arr[maxPos]) {
                maxPos = j;
            }
        }

        std::swap(arr[size - 1 - i], arr[maxPos]);
    }
}