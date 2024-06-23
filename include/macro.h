/*
 * @Author: Caviar
 * @Date: 2024-06-22 22:49:45
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-23 23:32:50
 * @Description: 
 */

#pragma once

#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;

#define ALLOW_COPY_AND_ASSIGN(ClassName) \
    ClassName(const ClassName&) = default; \
    ClassName& operator=(const ClassName&) = default;

#define DISALLOW_MOVE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;

#define ALLOW_MOVE(ClassName) \
    ClassName(ClassName&&) = default; \
    ClassName& operator=(ClassName&&) = default;

#define DISALLOW_COPY_AND_MOVE(ClassName) \
    DISALLOW_COPY_AND_ASSIGN(ClassName) \
    DISALLOW_MOVE(ClassName)

#define ALLOW_COPY_AND_MOVE(ClassName) \
    ALLOW_COPY_AND_ASSIGN(ClassName) \
    ALLOW_MOVE(ClassName)
