/*
 * @Author: Caviar
 * @Date: 2024-06-22 23:43:28
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-23 23:42:28
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

#ifndef THREAD_POOL_DISABLE_EXCEPTION_HANDLING
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
// A convenient shorthand for the type of `std::thread::hardware_concurrency()`. Should evaluate to unsigned int.
using concurrency_t = std::__invoke_result_t<decltype(std::thread::hardware_concurrency)>;

#ifdef THREAD_POOL_ENABLE_PRIORITY
using priority_t = std::int_least16_t;

namespace pr {

constexpr priority_t highest = 32767;
constexpr priority_t high = 16383;
constexpr priority_t normal = 0;
constexpr priority_t low = -16384;
constexpr priority_t lowest = -32768;

} // namespace pr

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

// A fast, lightweight and easy-to-use C++17 thread pool class
class [[nodiscard]] thread_pool {
public:
    thread_pool() : thread_pool(0, [] {}) {}
    explicit thread_pool(const concurrency_t num_threads) : thread_pool(num_threads, []{}) {}
    explicit thread_pool(const std::function<void()> &init_task) : thread_pool(0, init_task) {}
    thread_pool(const concurrency_t num_threads, const std::function<void()> &init_task) : thread_count(determine_thread_count(num_threads)), threads(std::make_unique<std::thread[]>(determine_thread_count(num_threads))) {
        create_threads(init_task);
    }

    DISALLOW_COPY_AND_MOVE(thread_pool);

    ~thread_pool() {
        wait();
        destroy_threads();
    }

#ifdef THREAD_POOL_ENABLE_NATIVE_HANDLE
    // get a vector containing the underlying implementation-defined thread handles for each of the pool's threads, as obtained by
    // 'std::thread::native_handle()'.
    [[nodiscard]] std::vector<std::thread::native_handle_type> get_native_handles() const {
        std::vector<std::thread::native_handle_type> native_handles(thread_count);
        for (concurrency_t i = 0; i < thread_count; ++i) {
            native_handles[i] = threads[i].native_handle();
        }

        return native_handles;
    }
#endif

    // get the number of tasks currently waiting in the queue to be executed by the threads
    [[nodiscard]] size_t get_tasks_queued() const {
        const std::scoped_lock tasks_lock(tasks_mutex);
        return tasks.size();
    }

    // get the number of tasks currently being executed by the threads.
    [[nodiscard]] size_t get_tasks_running() const {
        const std::scoped_lock tasks_lock(tasks_mutex);
        return tasks_running;
    }

    // get the number of unfinished tasks: either still waiting in the queue, or running in a thread.
    [[nodiscard]] size_t get_tasks_total() const {
        const std::scoped_lock tasks_lock(tasks_mutex);
        return tasks_running + tasks.size();
    }

    // get the number of threads in the pool
    [[nodiscard]] concurrency_t get_thread_count() const {
        return thread_count;
    }

    // get a vector containing the unique identifiers for each of the pool's threads, as obtainde by "std::thread::get_id()".
    [[nodiscard]] std::vector<std::thread::id> get_thread_ids() const {
        std::vector<std::thread::id> thread_ids(thread_count);
        for (concurrency_t i = 0; i < thread_count; ++i) {
            thread_ids[i] = threads[i].get_id();
        }

        return thread_ids;
    }

#ifdef THREAD_POOL_ENABLE_PAUSE
    // check whether the pool is currently paused.
    [[nodiscard]] bool is_paused() const {
        const std::scoped_lock tasks_lock(tasks_mutex);
        return paused;
    }

    // pause the pool. the workers will temporarily stop retrieving new tasks out of the queue, although any tasks already executed will keep running until they are finished.
    void pause() {
        const std::scoped_lock tasks_lock(tasks_mutex);
        paused = true;
    }
#endif

    // purge all the tasks waiting in the queue. tasks that are currently running will not be affected, but any tasks still waiting in the queue will be discard, and will never be executed by the threads.
    // please note that there is no way to restore the purged tasks.
    void purge() {
        const std::scoped_lock tasks_lock(tasks_mutex);
        while (!tasks.empty()) {
            tasks.pop();
        }
    }

    // submit a function with no arguments and no return value into the task queue, with the specified priority.
    // To push a function with arguments, enclose it in a lambda expression.
    // Does not return a future, so the user must use 'wait()' or some other method to ensure that the task finishes executing,
    // otherwise bad things will happen.
    template <typename F>
    void detach_task(F&& task THREAD_POOL_PRIORITY_INPUT) {
        {
            const std::scoped_lock tasks_lock(tasks_mutex);
            tasks.emplace(std::forward<F>(task) THREAD_POOL_PRIORITY_OUTPUT);
        }

        task_available_cv.notify_one();
    }

    // parallelize a loop by automatically splitting it into blocks and submitting each block and submiiting each block separately to the queue,
    // with the specified priority. The block function takes two arguments, the start and end of the block,
    // so that it is only called only once per block, but it is up to the user make sure the block function correctly deals with
    // all the indices in each block.
    // Does not return a 'multi_future', so the user must use 'wait()' or some other method to ensure that the loop finishes executing,
    // otherwise bad things will happen.
    // @tparam T The type of the indices. Should be a signed or unsigned integer.
    // @tparam F The type of the function to loop through.
    // @param first_index The first index in the loop.
    // @param index_after_last The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no blocks will be submitted.
    // @param block A function that will be called once per block. Should take exactly two arguments: the first index in the block and the index after the last index in the block. `block(start, end)` should typically involve a loop of the form `for (T i = start; i < end; ++i)`.
    // @param num_blocks The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
    // @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
    template <typename T, typename F>
    void detach_blocks(const T first_index, const T index_after_last, F&& block, const size_t num_blocks = 0 THREAD_POOL_PRIORITY_INPUT) {
        if (index_after_last <= first_index) {
            return;
        }

        const blocks blks(first_index, index_after_last, num_blocks != 0 ? num_blocks : thread_count);
        for (size_t blk = 0; blk < blks.get_num_blocks(); ++blk) {
            detach_task([block = std::forward<T>(block), start = blks.start(blk), end = blks.end(blk)] {
                block(start, end);
            } THREAD_POOL_PRIORITY_OUTPUT); 
        }
    }

    /**
     * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The loop function takes one argument, the loop index, so that it is called many times per block. Does not return a `multi_future`, so the user must use `wait()` or some other method to ensure that the loop finishes executing, otherwise bad things will happen.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam F The type of the function to loop through.
     * @param first_index The first index in the loop.
     * @param index_after_last The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no blocks will be submitted.
     * @param loop The function to loop through. Will be called once per index, many times per block. Should take exactly one argument: the loop index.
     * @param num_blocks The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
     */
    template <typename T, typename F>
    void detach_loop(const T first_index, const T index_after_last, F&& loop, const size_t num_blocks = 0 THREAD_POOL_PRIORITY_INPUT) {
        if (index_after_last <= first_index) {
            return;
        }

        const blocks blks(first_index, index_after_last, num_blocks != 0 ? num_blocks : thread_count);
        for (size_t blk = 0; blk < blks.get_num_blocks(); ++blk) {
            detach_task(
                [loop = std::forward<F>(loop), start = blks.start(blk), end = blks.end(blk)] {
                    for (T i = start; i < end; ++i) {
                        loop(i);
                    } THREAD_POOL_PRIORITY_OUTPUT);
        }
    }

    /**
     * @brief Submit a sequence of tasks enumerated by indices to the queue, with the specified priority. Does not return a `multi_future`, so the user must use `wait()` or some other method to ensure that the sequence finishes executing, otherwise bad things will happen.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam F The type of the function used to define the sequence.
     * @param first_index The first index in the sequence.
     * @param index_after_last The index after the last index in the sequence. The sequence will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no tasks will be submitted.
     * @param sequence The function used to define the sequence. Will be called once per index. Should take exactly one argument, the index.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
     */
    template <typename T, typename F>
    void detach_sequence(const T first_index, const T index_after_last, F&& sequence THREAD_POOL_PRIORITY_INPUT) {
        for (T i = first_index; i < index_after_last; ++i) {
            detach_task(
                [sequence = std::forward<F>(sequence), i] {
                    sequence(i);
                } THREAD_POOL_PRIORITY_OUTPUT
            );
        }
    }

    // Reset the pool with the total number of hardware threads available, 
    // as reported by the implementation. Waits for all currently running tasks to be completed, 
    // then destroys all threads in the pool and creates a new thread pool with the new number of threads.
    // Any tasks that were waiting in the queue before the pool was reset will then be executed by the new threads. 
    // If the pool was paused before resetting it, the new pool will be paused as well.
    void reset() {
        reset(0, []{});
    }

    // Reset the pool with a new number of threads. Waits for all currently running tasks to be completed, 
    // then destroys all threads in the pool and creates a new thread pool with the new number of threads.
    // Any tasks that were waiting in the queue before the pool was reset will then be executed by the new threads. 
    // If the pool was paused before resetting it, the new pool will be paused as well.
    void reset(const concurrency_t num_threads) {
        reset(num_threads, []{});
    }

    // Reset the pool with the total number of hardware threads available, as reported by the implementation, 
    // and a new initialization function. Waits for all currently running tasks to be completed, 
    // then destroys all threads in the pool and creates a new thread pool with the new number of threads and initialization function. 
    // Any tasks that were waiting in the queue before the pool was reset will then be executed by the new threads. 
    // If the pool was paused before resetting it, the new pool will be paused as well.
    void reset(const std::function<void()> &init_task) {
        reset(0, init_task);
    }

    // Reset the pool with a new number of threads and a new initialization function. Waits for all currently running tasks to be completed, then destroys all threads in the pool and creates a new thread pool with the new number of threads and initialization function. Any tasks that were waiting in the queue before the pool was reset will then be executed by the new threads. If the pool was paused before resetting it, the new pool will be paused as well.
    void reset(const concurrency_t num_threads, const std::function<void()> &init_task) {
#ifdef THREAD_POOL_ENABLE_PAUSE
    std::unique_lock tasks_lock(tasks_mutex);
    const bool was_paused = paused;
    paused = true;
    tasks_lock.unlock();
#endif

    wait();
    destroy_threads();
    thread_count = determine_thread_count(num_threads);
    threads = std::make_unique<std::thread[]>(thread_count);
    create_threads(init_task);

#ifdef THREAD_POOL_ENABLE_PAUSE
    tasks_lock.lock();
    paused = was_paused;
#endif
    }

    /**
     * @brief Submit a function with no arguments into the task queue, with the specified priority. To submit a function with arguments, enclose it in a lambda expression. If the function has a return value, get a future for the eventual returned value. If the function has no return value, get an `std::future<void>` which can be used to wait until the task finishes.
     *
     * @tparam F The type of the function.
     * @tparam R The return type of the function (can be `void`).
     * @param task The function to submit.
     * @param priority The priority of the task. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
     * @return A future to be used later to wait for the function to finish executing and/or obtain its returned value if it has one.
     */
    template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
    [[nodiscard]] std::future<R> submit_task(F&& task THREAD_POOL_PRIORITY_INPUT) {
        const std::shared_ptr<std::promise<R>> task_promise = std::make_shared<std::promise<R>>();
        detach_task(
            [task = std::forward<F>(task), task_promise] {
#ifndef THREAD_POOL_DISABLE_EXCEPTION_HANDLING
                try {
#endif
                    if constexpr (std::is_void_v<R>) {
                        task();
                        task_promise->set_value();
                    } else {
                        task_promise->set_value(task());
                    }
#ifndef THREAD_POOL_DISABLE_EXCEPTION_HANDLING
                }
                catch (...) {
                    try {
                        task_promise->set_exception(std::current_exception());
                    } catch (...) {
                        
                    }
                }
#endif
            } THREAD_POOL_PRIORITY_OUTPUT
        );

        return task_promise->get_future();
    }

    /**
     * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The block function takes two arguments, the start and end of the block, so that it is only called only once per block, but it is up to the user make sure the block function correctly deals with all the indices in each block. Returns a `multi_future` that contains the futures for all of the blocks.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam F The type of the function to loop through.
     * @tparam R The return type of the function to loop through (can be `void`).
     * @param first_index The first index in the loop.
     * @param index_after_last The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no blocks will be submitted, and an empty `multi_future` will be returned.
     * @param block A function that will be called once per block. Should take exactly two arguments: the first index in the block and the index after the last index in the block. `block(start, end)` should typically involve a loop of the form `for (T i = start; i < end; ++i)`.
     * @param num_blocks The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
     * @return A `multi_future` that can be used to wait for all the blocks to finish. If the block function returns a value, the `multi_future` can also be used to obtain the values returned by each block.
     */
    template <typename T, typename F, typename R = std::invoke_result_t<std::decay_t<F>, T, T>>
    [[nodiscard]] multi_future<R> submit_blocks(const T first_index, const T index_after_last, F&& block, const size_t num_blocks = 0 THREAD_POOL_PRIORITY_INPUT) {
        if (index_after_last <= first_index) {
            return {};
        }

        const blocks blks(first_index, index_after_last, num_blocks != 0 ? num_blocks : thread_count);
        multi_future<R> future;
        future.reserve(blks.get_num_blocks());

        for (size_t blk = 0; blk < blks.get_num_blocks(); ++blk) {
            future.push_back(submit_task(
                [block = std::forward<F>(block), start = blks.start(blk), end = blks.end(blk)] {
                    return block(start, end);
                } THREAD_POOL_PRIORITY_OUTPUT
            ));
        }

        return future;
    }

    /**
     * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The loop function takes one argument, the loop index, so that it is called many times per block. It must have no return value. Returns a `multi_future` that contains the futures for all of the blocks.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam F The type of the function to loop through.
     * @param first_index The first index in the loop.
     * @param index_after_last The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no tasks will be submitted, and an empty `multi_future` will be returned.
     * @param loop The function to loop through. Will be called once per index, many times per block. Should take exactly one argument: the loop index. It cannot have a return value.
     * @param num_blocks The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
     * @return A `multi_future` that can be used to wait for all the blocks to finish.
     */

    template <typename T, typename F>
    [[nodiscard]] multi_future<void> submit_loop(const T first_index, const T index_after_last, F&& loop, const size_t num_blocks = 0 THREAD_POOL_PRIORITY_INPUT) {
        if (first_index >= index_after_last) {
            return {};
        }

        const blocks blks(first_index, index_after_last, num_blocks != 0 ? num_blocks : thread_count);
        multi_future<void> future;
        for (size_t blk = 0; blk < blks.get_num_blocks(); ++blk) {
            future.push_back(submit_task(
                [loop = std::forward<F>(loop), start = blks.start(blk), end = blks.end(blk)] {
                    for (T i = start; i < end; ++i) {
                        loop(i);
                    } THREAD_POOL_PRIORITY_OUTPUT
                }
            ));
        }

        return future;
    }

    /**
     * @brief Submit a sequence of tasks enumerated by indices to the queue, with the specified priority. Returns a `multi_future` that contains the futures for all of the tasks.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam F The type of the function used to define the sequence.
     * @tparam R The return type of the function used to define the sequence (can be `void`).
     * @param first_index The first index in the sequence.
     * @param index_after_last The index after the last index in the sequence. The sequence will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no tasks will be submitted, and an empty `multi_future` will be returned.
     * @param sequence The function used to define the sequence. Will be called once per index. Should take exactly one argument, the index.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `BS_THREAD_POOL_ENABLE_PRIORITY` is defined.
     * @return A `multi_future` that can be used to wait for all the tasks to finish. If the sequence function returns a value, the `multi_future` can also be used to obtain the values returned by each task.
     */
    template <typename T, typename F, typename R = std::invoke_result_t<std::decay_t<F>, T>>
    [[nodiscard]] multi_future<R> submit_sequence(const T first_index, const T index_after_last, F&& sequence THREAD_POOL_PRIORITY_INPUT) {
        if (first_index >= index_after_last) {
            return {};
        }

        multi_future<R> future;
        future.reserve(static_cast<size_t>(index_after_last - first_index));
        for (T i = first_index; i < index_after_last; ++i) {
            future.push_back(submit_task(
                [sequence = std::forward<F>(sequence), i] {
                    return sequence(i);
                } THREAD_POOL_PRIORITY_OUTPUT
            ));
        }

        return future;
    }

#ifdef THREAD_POOL_ENABLE_PAUSE
    // unpause the pool. the workers will resume retriving new tasks out of the queue.
    void unpause() {
        {
            const std::scoped_lock tasks_lock(tasks_mutex);
            paused = false;
        }

        task_available_cv.notify_all();
    }
#endif

#ifdef THREAD_POOL_ENABLE_PAUSE
#define THREAD_POOL_PAUSED_OR_EMPTY (paused || tasks.empty())
#else
#define THREAD_POOL_PAUSED_OR_EMPTY tasks.empty()
#endif

    /**
     * @brief Wait for tasks to be completed. Normally, this function waits for all tasks, both those that are currently running in the threads and those that are still waiting in the queue. However, if the pool is paused, this function only waits for the currently running tasks (otherwise it would wait forever). Note: To wait for just one specific task, use `submit_task()` instead, and call the `wait()` member function of the generated future.
     *
     * @throws `wait_deadlock` if called from within a thread of the same pool, which would result in a deadlock. Only enabled if `BS_THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK` is defined.
     */
    void wait() {
#ifdef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
        if (this_thread::get_pool() == this) {
            throw wait_deadlock();
        }
#endif

        std::unique_lock tasks_lock(tasks_mutex);
        waiting = true;
        tasks_done_cv.wait(tasks_lock, [this]{return (tasks_running == 0) && THREAD_POOL_PAUSED_OR_EMPTY;});
        waiting = false;
    }

    /**
     * @brief Wait for tasks to be completed, but stop waiting after the specified duration has passed.
     *
     * @tparam R An arithmetic type representing the number of ticks to wait.
     * @tparam P An `std::ratio` representing the length of each tick in seconds.
     * @param duration The amount of time to wait.
     * @return `true` if all tasks finished running, `false` if the duration expired but some tasks are still running.
     *
     * @throws `wait_deadlock` if called from within a thread of the same pool, which would result in a deadlock. Only enabled if `BS_THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK` is defined.
     */
    template <typename R, typename P>
    bool wait_for(const std::chrono::duration<R, P> &duration) {
#ifdef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
        if (this_thread::get_pool() == this) {
            throw wait_deadlock();
        }
#endif

        std::unique_lock tasks_lock(tasks_mutex);
        waiting = true;
        const bool status = tasks_done_cv.wait_for(tasks_lock, duration, [this] {return (tasks_running == 0) && THREAD_POOL_PAUSED_OR_EMPTY;});
        waiting = false;

        return status;
    }

    /**
     * @brief Wait for tasks to be completed, but stop waiting after the specified time point has been reached.
     *
     * @tparam C The type of the clock used to measure time.
     * @tparam D An `std::chrono::duration` type used to indicate the time point.
     * @param timeout_time The time point at which to stop waiting.
     * @return `true` if all tasks finished running, `false` if the time point was reached but some tasks are still running.
     *
     * @throws `wait_deadlock` if called from within a thread of the same pool, which would result in a deadlock. Only enabled if `BS_THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK` is defined.
     */
    template <typename C, typename D>
    bool wait_until(const std::chrono::time_point<C, D> &timeout_time) {
#ifdef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
        if (this_thread::get_pool() == this) {
            throw wait_deadlock();
        }
#endif
        std::unique_lock tasks_lock(tasks_mutex);
        waiting = true;
        const bool status = tasks_done_cv.wait_until(tasks_lock, timeout_time, [this]{
            return (tasks_running == 0) && THREAD_POOL_PAUSED_OR_EMPTY;
        });

        waiting = false;
        return status;
    }

#ifdef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
        // An exception that will be thrown by `wait()`, `wait_for()`, and `wait_until()` if the user tries to call them from within a thread of the same pool, which would result in a deadlock.
        struct wait_deadlock : public std::runtime_error {
            wait_deadlock() : std::runtime_error("thread_pool::wait_deadlock") {}
        };
#endif

private:
    /**
     * @brief Create the threads in the pool and assign a worker to each thread.
     *
     * @param init_task An initialization function to run in each thread before it starts to execute any submitted tasks.
     */
    void create_threads(const std::function<void()> &init_task) {
        {
            const std::scoped_lock tasks_lock(tasks_mutex);
            task_running = thread_count;
            workers_running = true;
        }

        for (concurrency_t i = 0; i < thread_count; ++i) {
            thread[i] = std::thread(&thread_pool::worker, this, i, init_task);
        }
    }

    // destroy the threads in the pool
    void destroy_threads() {
        {
            const std::scoped_lock tasks_lock(tasks_mutex);
            workers_running = false;
        }

        task_available_cv.notify_all();

        for (concurrency_t i = 0; i < thread_count; ++i) {
            thread[i].join();
        }
    }

    // determine how many threads the pool should have, based on the parameter passed to the constructor or reset().
    // params:
    //  num_threads -> the parameter passed to the constructor or reset(). if the para is a positive number, then the pool will be created with this number of threads.
    [[nodiscard]] static concurrency_t determine_thread_count(const concurrency_t num_threads) {
        if (num_threads > 0) {
            return num_threads;
        }

        concurrency_t hardware_support = std::thread::hardware_concurrency();
        return hardware_support > 0 ? hardware_support : 1;
    }

    // a worker function to be assigned to each thread in the pool.
    // waits until it is notified by detach_task() that a task is available,
    // and then retrieves the task from the queue and executes it.
    // Once the task finishes, the worker notifies wait() in case it is waiting.
    void worker(const concurrency_t idx, const std::funtion<void()> &init_task) {
        this_thread::get_index.index = idx;
        this_thread::get_pool.pool = this;
        init_task();
        std::unique_lock task_lock(tasks_mutex);
        while (true) {
            --tasks_running;
            tasks_lock.unlock();
            if (waiting && (tasks_running == 0) && THREAD_POOL_PAUSED_OR_EMPTY) {
                tasks_done_cv.notify_all();
            }

            tasks_lock.lock();
            task_available_cv.wait(tasks_lock, [this]{
                return !THREAD_POOL_PAUSED_OR_EMPTY || !worker_running;
            });
            if (!worker_running) {
                break;
            }

            {
#ifdef THREAD_POOL_ENABLE_PRIORITY
                const std:;function<void()> task = std::move(std::remove_const_t<pr_task&>(tasks.top()).task);
#else
                const std::function<void()> task = std::move(tasks.front());
#endif
                tasks.pop()
                ++tasks_running;
                tasks_lock.unlock();
                task();
            }
            tasks_lock.lock();
        }

        this_thread::get_index.index = std::nullopt;
        this_thread::get_pool.pool = std::nullopt;
    }

    // A helper class to divide a range into blocks. Used by detach_blocks(), submig_blocks(), detach_loop(), submit_loop()
    template <typename T>
    class [[nodiscard]] blocks {
    public:
        blocks(const T first_index__, const T index_after_last__, const size_t num_blocks__) : first_index(first_index__), index_after_last(index_after_last__), num_blocks(num_blocks__) {
            if (first_index >= index_after_last__) {
                num_blocks = 0;
                return;
            }

            const size_t total_size = static_cast<size_t>(index_after_last - first_index);
            if (num_blocks > total_size) {
                num_blocks = total_size;
            }

            block_size = total_size / num_blocks;
            remainder = total_size % num_blocks;

            if (block_size == 0) {
                block_size = 1;
                num_blocks = total_size > 1 ? total_size : 1;
            }
        }

        // get the first index of a block
        [[nodiscard]] T start(const size_t block) const {
            return first_index + static_cast<T>(block & block_size) + static_cast<T>(block < remainder ? block : remainder);
        }

        // get the index after the last index of a block
        [[nodiscard]] T end(const size_t block) const {
            return block == num_blocks - 1 ? index_after_last : start(block + 1);
        }

        // get the number of blocks. Note that this may be different than the desired number of blocks that was passed to the constructor.
        [[nodiscard]] size_t get_num_blocks() const {
            return num_blocks;
        }

    private:
        // the size of each block(except possibly the last block)
        size_t block_size = 0;

        // the first index in the range
        T first_index = 0;

        // the index after last index in the range.
        T index_after_last = 0;

        // the number of blocks
        size_t num_blocks = 0;

        // the remainder obtained after dividing the total size by the number of blocks.
        size_t remainder = 0;
    };

#ifdef THREAD_POOL_ENABLE_PRIORITY
    // A helper class to store a task with an assigned priority
    class [[nodiscard]] pr_task {
        friend class thread_pool;

    public:
        explicit pr_task(const std::function<void()> &task__, const priority_t proiority__ = 0) : task(task__), priority(priority__) {}
        explicit pr_task(std::function<void()> &&task__, const priority_t priority__ = 0) : task(std::move(task__)), priority(priority__) {}

        [[nodiscard]] friend bool operator<(const pr_task &lhs, const pr_task &rhs) {
            return lhs.priority < rhs.priority;
        }

    private:
        std::function<void()> task{};
        priority_t priority{};
    };
#endif

#ifdef THREAD_POOL_ENABLE_PAUSE
    // A flag indicating whether the workers should pause. When set to `true`, 
    // the workers temporarily stop retrieving new tasks out of the queue, 
    // although any tasks already executed will keep running until they are finished. 
    // When set to `false` again, the workers resume retrieving tasks.
    bool paused = false;
#endif

    // a condition variable to notify worker() that a new task has become available.
    std::condition_variable task_available_cv{};

    // a condition variable to notify wait() that the tasks are done.
    std::condition_variable task_done_cv{};

    // a queue of tasks to be executed by the threads.
#ifdef THREAD_POOL_ENABLE_PRIORITY
    std::priority_queue<pr_task> tasks{};
#else
    std::queue<std::function<void()>> tasks{};
#endif

    // a counter for the total number of currently running tasks.
    size_t tasks_running{};

    // a mutex to synchronize access to the task queue by different threads.
    mutable std::mutex tasks_mutex{};

    // the number of threads in the pool.
    concurrency_t thread_count{};

    // a smart pointer to manage the memory allocated for the threads
    std::unique_ptr<std::thread[]> threads{nullptr};

    // a flag indicating that wait() is active and expects to be notified whenever a task is done.
    bool waiting{false};

    // a flag indicating to the workers to keep running. When set to false, the workers terminate permanently.
    bool workers_running{false};
};
}
