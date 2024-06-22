<!--
 * @Author: Caviar
 * @Date: 2024-06-21 23:09:10
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 15:03:28
 * @Description: 
-->

# Sort Algorithm

## 1 简介

| 排序算法 | 平均时间复杂度 | 最好 | 最坏 | 空间复杂度 | 排序方式 | 稳定性 |
|   ----  | ----        | ---- | ---- | ----     |  ---- | ---- |
| 冒泡排序 | O(n^2)      | O(n) | O(n^2) | O(1) | In place | stable |
| 选择排序 | O(n^2)      | O(n^2) | O(n^2) | O(1) | In place | non-stable |
| 插入排序 | O(n^2)      | O(n) | O(n^2) | O(1) | In place | stable |
| 希尔排序 | O(nlgn)     | O(nlg^2(n)) | O(nlg^2(n)) | O(1) | In place | non-stable |
| 归并排序 | O(nlgn)     | O(nlgn) | O(nlgn) | O(n) | Out place | stable |
| 快速排序 | O(nlgn)     | O(nlgn) | O(n^2) | O(lgn) | In place | non-stable |
| 堆排序  | O(nlgn)     | O(nlgn) | O(nlgn) | O(1) | In place | non-stable |
| 计数排序 | O(n + k)    | O(n + k) | O(n + k) | O(k) | Out place | stable |
| 桶排序  | O(n + k)    | O(n + k) | O(n^2) | O(n + k) | Out place | stable |
| 计数排序 | O(n * k)    | O(n * k) | O(n * k) | O(n + k) | Out place | stable |

[注]
k："桶"的个数
In-place：占用常数内存，不占用额外内存
Out-place：占用额外内存

