/*
 * @Author: Caviar
 * @Date: 2024-06-22 17:04:16
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-22 17:50:58
 * @Description: 
 */

#pragma once

#include <string>
#include <unordered_map>

/*
    算法描述:
        hash记录字符最后一次出现的位置
        左指针更新: 根据上轮左指针和hash表情况，每轮更新，保证区间[left + 1, right]内无重复字符且最大
        结果: 取上轮计算最大值与本轮双指针区间中最大值
*/
int GetLengthOfLongestSubstring(const std::string &s) {
    auto size = s.length();
    int left{-1};
    int res{};
    std::unordered_map<char, int> cache{};
    for (int right = 0; right < size; ++right) {
        auto it = cache.find(s[right]); // 查找当前缓存中是否有相同元素
        if (it != cache.end()) { // 有相同元素情况
            left = std::max(left, it->second); // 更新左边界
        }

        cache[s[right]] = right; // 插入元素新位置
        res = std::max(res, right - left);
    }

    return res;
}
