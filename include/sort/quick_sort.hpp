/*
 * @Author: Caviar
 * @Date: 2024-06-20 13:48:31
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-21 23:04:40
 * @Description: 快排算法
 * 基本思想：
 *   1 从数组中取出一个数作为基准数
 *   2 分区，以基准数为中心，左边小于等于基准数，右边大于基准数 (分治思想)
 *   3 重复递归处理
 */

#pragma once

#include <vector>

template <typename T>
void QuickSort(std::vector<T> &arr, int left, int right) // 闭区间
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
