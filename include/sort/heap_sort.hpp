/*
 * @Author: Caviar
 * @Date: 2024-06-22 15:34:24
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 15:43:13
 * @Description: 堆排序
 * 基本思想：利用堆（近似完全二叉树）结构，以及其性质：子结点的键值总小于或大于其父结点
 * 大顶堆/小顶堆
 * 
 * 算法步骤：
 *  1 创建一个堆H[0...n-1]
 *  2 把堆首（最大值）与堆尾互换；
 *  3 把堆的尺寸减一，调用shift_down(0),目的是将新的数组顶端调整到相应位置
 *  4 重复步骤2,直到堆尺寸为1
 */

#pragma once

#include <vector>

template <typename T>
void MaxHeapify(std::vector<T> &arr, int start, int end)
{
    int father = start;
    int son = (father << 1) + 1;
    while (son <= end) {
        if (son + 1 <= end && arr[son] < arr[son + 1]) {
            son++;
        }

        if (arr[father] > arr[son]) {
            return;
        }

        std::swap(arr[father], arr[son]);
        father = son;
        son = (father << 1) + 1;
    }
}

template <typename T>
void HeapSort(std::vector<T> &arr)
{
    auto size = arr.size();

    for (int i = size >> 1 - 1; i >= 0; --i) {

    }
}
