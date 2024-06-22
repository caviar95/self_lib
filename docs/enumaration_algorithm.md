# Enumeration Algorithm

## 1 简介

枚举算法就是我们常用的穷举法。

核心思想：列举问题的所有状态，逐一比较，得到最终解

适用范围：求解问题规模比较小的问题，或者作为子过程处理算法

## 2 解题思路

1 确定枚举对象/范围/判断条件；
2 遍历所有可能情况，验证是否有效；
3 优化算法；

优化思路：
1 抓住问题状态的本质，尽可能缩小问题状态空间大小；
2 加强约束条件，缩小枚举范围；
3 根据某些问题特质，eg.对称性等，避免重复求解；

## 3 示例

百钱买百鸡：
公鸡一只五块钱，母鸡一只三块钱，小鸡三只一块钱。现在我们用100块钱买了100只鸡，问公鸡、母鸡、小鸡各买了多少只？

步骤1：确定枚举对象/范围/判断条件
    枚举对象：公鸡/母鸡/小鸡数量分别为x, y, z
    枚举范围：0 <= x,y,z <= 100
    判断条件：5 * x + 3 * y + z / 3 = 100; x + y + z = 100;
步骤2：遍历并验证
```cpp
void GetChickenNums() {
    for (auto i = 0; i <= 100; ++i) {
        for (auto j = 0; j <= 100; ++j) {
            for (auto k = 0; k <= 100, ++k) {
                if (k % 3 != 0) {
                    continue;
                }

                if (i + j + k != 100) {
                    continue;
                }

                if (i * 5 + j * 3 + z / 3 != 100) {
                    continue;
                }

                std::cout << "result: i=" << i << ", j=" << j << ", k=", k << std::endl;
            }
        }
    }
}
```

步骤3：优化
1 z通过100 - x - y表示，减少参数数量；
2 公鸡最多20只，母鸡最多33只；

```cpp
void GetChickenNums() {
    for (auto i = 0; i <= 20; ++i) {
        for (auto j = 0; j <= 33; ++j) {
            auto k = 100 - i - j;
            if (k % 3 != 0) {
                continue;
            }

            if (i * 5 + j * 3 + z / 3 != 100) {
                continue;
            }

            std::cout << "result: i=" << i << ", j=" << j << ", k=", k << std::endl;
        }
    }
}

## 4 leetcode实战

### 4.1 两数之和

1 确定枚举
    x + y = target
2 遍历
```cpp
class Solution {
public:
    std::vector<int> twoSum(const std::vector<int> &arr, int target) {
        for (auto i = 0; i < arr.size(); ++i) {
            for (auto j = i + 1; j < arr.size(); ++j) {
                if (arr[i] + arr[j] == target) {
                    return {i, j};
                }
            }
        }

        return {};
    }
};
```

3 优化

思路：
    1 y = target - x, 减少未知因素
    2 hash缓存
```cpp
class Solution {
public:
    std::vector<int> twoSum(const std::vector<int> &arr, int target) {
        std::unordered_map<int, int> cache{};

        for (auto i = 0; i < arr.size(); ++i) {
            auto it = cache.find(target - arr[i]);
            if (it != cache.end()) {
                return {it->second, i};
            }

            cache.emplace(arr[i], i);
        }

        return {};
    }
};

```

### 4.2 计数质数

1 确定枚举
    [0, n)
2 遍历
```cpp
// 暴力超时
class Solution {
public:
    int countPrimes(int n) {
        if (n <= 1) {
            return 0;
        }

        int cnt = 0;
        for (auto i = 2; i < n; ++i) {
            if (isPrime(i)) {
                cnt++;
            }
        }

        return cnt;
    }

private:
    bool isPrime(int x) {
        for (auto i = 2; i < x; ++i) {
            if (x % i == 0) {
                return false;
            }
        }

        return true;
    }
};
```