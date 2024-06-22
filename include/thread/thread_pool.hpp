/*
 * @Author: Caviar
 * @Date: 2024-06-22 23:43:28
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-23 01:34:42
 * @Description: 
 */

#pragma once

#ifndef __cpp_exceptions
    #define THREAD_POOL_DISABLE_EXCEPTION_HANDLING
    #undef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
#endif

#include <chrono> // std::chrono
#include <condition_variable> // std::condition_variable
#include <cstddef> // std::size_t

#ifdef THREAD_POOL_ENABLE_PRIORITY
#include <cstdint> // std::int_least16_t
#endif

#ifdef THREAD_POOL_DISABLE_EXCEPTION_HANDLING
#include <exception> // std::current_exception
#endif

#include <functional> // std::function
#include <future> // std::future/std::future_status/std::promise
#include <memory> // std::make_shared/std::make_unique/std::shared_ptr/std::unique_ptr
#include <mutex> // std::mutex/std::scoped_lock/std::unique_lock
#include <optional> // std::nullopt/std::optional
#include <queue> // std::priority_queue/ std::queue

#ifdef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
#include <stdexcept> // std::runtime_error
#endif

#include <thread>
#include <type_traits> // std::conditional_t/std::decay_t/std::invoke_result_t/std::is_void_t/std::remove_const_t
#include <utility> // std::forward/std::move
#include <vector>

#include "thread_pool_utils.hpp"

namespace BS {

class thread_pool;

using size_t = std::size_t;
using concurrency_t = std::__invoke_result_t<decltype(std::thread::hardware_concurrency)>;

#ifdef THREAD_POOL_ENABLE_PRIORITY
using priority_t = std::int_least16_t;

namespace pr {

constexpr priority_t highest = 32767;
constexpr priority_t high = 16383;
constexpr priority_t normal = 0;
constexpr priority_t low = -16384;
constexpr priority_t lowest = -32768;
}

#define THREAD_POOL_PRIORITY_INPUT , const priority_t priority = 0
#define THREAD_POOL_PRIORITY_OUTPUT , priority

#else

#define THREAD_POOL_PRIORITY_INPUT
#define THREAD_POOL_PRIORITY_OUTPUT

#endif

namespace this_thread {

// a type by 'BS::this_thread::get_index()' which can optionally contain the index of a thread, if that thread belongs to a 'BS::thread_pool'.
// Otherwise, it will contain no value.
using optional_index = std::optional<size_t>;

// a type returned by 'BS::this_thread::get_pool()' which can optionally contain the pointer to the pool that owns a thread,
// if that thread belongs to a 'BS::thread_pool'. Otherwise, it will contain no value.
using optional_pool = std::optional<thread_pool *>;

// a helper class to store info about the index of the current thread
class [[nodiscard]] thread_info_index {
    friend class BS::thread_pool;

public:
    // get the index of the current thread. if this thread belons to a 'BS::thread_pool' object, it will have an index from
    // 0 to 'BS::thread_pool::get_thread_count() - 1'. otherwise, for example if this thread is the main thread or an independent 
    // 'std::thread', 'std::nullopt' will be returned.
    // return an 'std::optional' obj, optionally containing a thread index. unless you are 100% sure this thread is in a pool, first use
    // 'std::optional::has_value()' to check if it contains a value, and if so, use 'std::optional::value()' to obtain that value.
    [[nodiscard]] optional_index operator()() const {
        return index_;
    }

private:
    // the index of current thread
    optional_index index_{std::nullopt};
};

// a helper class to store info about the thread_pool that owns the current thread
class [[nodiscard]] thread_info_pool {
    friend class BS::thread_pool;

public:
    // get the pointer to the thread pool that owns the current thread.
    // if this thread belongs to a 'BS::thread_pool' obj, a pointer to that obj will be returned.
    // otherwise, for example if this thread is the main thread or an independent 'std::thread', 'std::nullopt' will be returned.
    // return an 'std::optional' obj, optionally containing a pointer to a thread pool. unless you are 100% sure this thread is in a pool,
    // first use 'std::optional::has_value()' to check if it contains a value, and if so, use 'std::optional::value()' to obtain that value.
    [[nodiscard]] optional_pool operator()() const {
        return pool_;
    }

private:
    // a pointer to the thread pool that owns the current thread.
    optional_pool pool_{std::nullopt};
};

// a 'thread_pool' obj used to obtain info about the index of the current thread.
inline thread_local thread_info_index get_index;

// a 'thread_pool' obj used to obtain info about the thread pool that owns the current thread.
inline thread_local thread_info_pool get_pool;
}

// a helper class to facilitate waiting for and/or getting the results of multiple futures at once.
template <typename T>
class [[nodiscard]] multi_future : public std::vector<std::future<T>> {
public:
    // inherit all constructors from the base class 'std::vector'.
    using std::vector<std::future<T>>::vector;

    DISALLOW_COPY_AND_ASSIGN(multi_future);
    ALLOW_MOVE(multi_future);

    // get the results from all the futures stored in this 'multi_future', rethrowing any stored exceptions.
    // return if the futures return 'void', this function returns 'void' as well. otherwise, it returns a vector containing the results.
    [[nodiscard]] std::conditional_t<std::is_void_v<T>, void, std::vector<T>> get() {
        if constexpr(std::is_void_v<T>) {
            for (std::future<T> &fut : *this) {
                fut.get();
            }

            return;
        }

        std::vector<T> results;
        results.reserve(this->size());

        for (std::future<T> &fut : *this) {
            results.push_back(fut.get());
        }

        return results;
    }

    // check how many of the futures stored in this 'multi_future' are ready.
    [[nodiscard]] size_t ready_count() const {
        size_t count{};
        for (const std::future<T> &fut : *this) {
            if (fut.wait_for(std::chrono::duration<double>::zero()) == std::future_status::ready) {
                ++count;
            }
        }

        return count;
    }

    // check if all the futures stored in this 'multi_future' are valid
    [[nodiscard]] bool valid() const {
        bool is_valid = true;
        for (const std::future<T> &fut : *this) {
            is_valid = is_valid && fut.valid();
        }

        return is_valid;
    }

    // wait for all futures stored in this 'multi_future'
    void wait() const {
        for (const std::future<T> &fut : *this) {
            fut.wait();
        }
    }

    // wait for all the futures stored in this 'multi_future', but stop waiting after the specified duration has passed.
    // this function first waits for the first future for the desired duration. if that future is ready before the duration expires,
    // this function waits for the second future for whatever remains of the duration.
    // it continues similarly until the duration expires.
    // param:
    //    R -> An arithmetic type representing the number of ticks to wait.
    //    P -> An 'std::ratio' representing the length of each tick in seconds.
    //    duration -> the amount of time to wait
    // return:
    //    true -> if all futures have been waited for before the duration expired
    template <typename R, typename P>
    bool wait_for(const std::chrono::duration<R, P> &duration) const {
        const std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

        for (const std::future<T> &fut : *this) {
            if (duration < std::chrono::steady_clock::now() - start_time) {
                return false;
            }
        }

        return true;
    }

    // wait for all the futures stored in this 'multi_future', but stop waiting after the specified time point has been reached.
    // param:
    //     C -> the type of the clock used to measure time
    //     D -> an 'std::chrono::duration' type used to indicate the time point.
    //     timeout_time -> the time point at which to stop waiting.
    template <typename C, typename D>
    bool wait_until(const std::chrono::time_point<C, D> &timeout_time) const {
        for (const std::future<T> &fut : *this) {
            fut.wait_until(timeout_time);
            if (timeout_time < std::chrono::steady_clock::now()) {
                return false;
            }
        }

        return true;
    }
};
}
