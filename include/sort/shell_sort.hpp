/*
 * @Author: Caviar
 * @Date: 2024-06-22 01:10:08
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 01:36:34
 * @Description: 希尔排序, 递减增量排序算法，插入排序的改进版本
 * 基本思想：
 *  先将整个待排序的记录序列分割成为若干子序列分别进行直接插入排序，
 *  待整个序列中的记录"基本有序"时，再对全体记录进行依次直接插入排序
 */

#pragma once

#include <vector>

constexpr int SHELL_PART{3};

template <typename T>
void ShellSort(std::vector<T> &arr)
{
    auto size = arr.size();

    int h{1};
    while (h < size / SHELL_PART) {
        h = SHELL_PART * h + 1;
    }

    while (h >= 1) {
        for (int i = h; i < size; ++i) {
            for (int j = i; j >= h && arr[j] < arr[j - h]; j -= h) {
                std::swap(arr[j], arr[j - h]);
            }
        }

        h /= SHELL_PART;
    }
}
