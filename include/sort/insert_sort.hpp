/*
 * @Author: Caviar
 * @Date: 2024-06-22 00:48:51
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 01:33:07
 * @Description: 插入排序
 * 基本思想：遍历，将当前值取出，与前半段有序区依次比较，找到合适位置
 */

#pragma once

#include <vector>

template <typename T>
void InsertSort(std::vector<T> &arr)
{
    auto size = arr.size();

    for (auto i = 1; i < size; ++i) {
        auto key = arr[i];
        int j = i - 1;
        while (j >= 0 && key < arr[j]) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}
