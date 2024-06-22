/*
 * @Author: Caviar
 * @Date: 2024-06-22 14:50:44
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 15:26:24
 * @Description: 归并排序
 * 基本思想：分治
 * 实现方法：1 自上而下； 2 自下而上
 * 
 * 算法步骤：
 *  1 申请空间，大小未两个已排序序列之和，存放合并后的序列
 *  2 两个指针，分别指向两个序列
 *  3 比较两个指向元素，按照顺序存放到合并空间
 *  4 重复步骤3，直到某一指针到尾部
 *  5 将剩余元素移到合并队列队尾
 */

#pragma once

#include <vector>

// C版本
#if 0
template <typename T>
void MergeSortRecursive(std::vector<T> &arr, std::vector<T> &reg, int start, int end)
{
    if (start >= end) {
        return;
    }

    int len = end - start;
    int mid = (len >> 1) + start;
    int start1 = start;
    int end1 = mid;
    int start2 = mid + 1;
    int end2 = end;

    MergeSortRecursive(arr, reg, start1, end1);
    MergeSortRecursive(arr, reg, start2, end2);

    int k = start;

    while (start1 <= end1 && start2 <= end2) {
        reg[k++] = arr[start1] < arr[start2] ? arr[start1++] : arr[start2++];
    }

    while (start1 <= end1) {
        reg[k++] = arr[start1++];
    }

    while (start2 <= end2) {
        reg[k++] = arr[start2++];
    }

    for (k = start; k <= end; k++) {
        arr[k] = reg[k];
    }
}

template <typename T>
void MergeSort(std::vector<T> &arr)
{
    auto size = arr.size();
    std::vector<T> reg(size);
    MergeSortRecursive(arr, reg, 0, size - 1);
}
#endif

template <typename T>
void Merge(std::vector<T> &arr, int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = 0;

    std::vector<T> temp(right - left + 1); // 申请基于当前区段的额外空间

    while (i <= mid && j <= right) {
        temp[k++] = arr[i] <= arr[j] ? arr[i++] : arr[j++];
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (i = left, k = 0; i <= right; ++i, ++k) {
        arr[i] = temp[k];
    }
}

template <typename T>
void MergeSort(std::vector<T> &arr, int left, int right)
{
    if (left >= right) {
        return;
    }

    int mid = left + ((right - left) >> 1);
    MergeSort(arr, left, mid);
    MergeSort(arr, mid + 1, right);
    Merge(arr, left, mid, right);
}

template <typename T>
void MergeSort(std::vector<T> &arr)
{
    MergeSort(arr, 0, arr.size() - 1);
}
