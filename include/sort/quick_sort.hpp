/*
 * @Author: Caviar
 * @Date: 2024-06-20 13:48:31
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-21 13:43:24
 * @Description: 
 */

#pragma once

#include <vector>

template <typename T>
void QuickSort(std::vector<T> &arr, int left, int right)
{
    if (left >= right) {
        return;
    }

    int i{left};
    int j{right};
    T x = arr[left];

    while (i < j) {
        while (i < j && arr[j] >= x) { // 从右向左找第一个小于x的值
            j--;
        }

        if (i < j) {
            arr[i++] = arr[j];
        }

        while (i < j && arr[i] < x) { // 从左向右寻找第一个大于x的值
            i++;
        }

        if (i < j) {
            arr[j--] = arr[i];
        }

        arr[i] = x;

        QuickSort(arr, left, i - 1);
        QuickSort(arr, i + 1, right);
    }
}
