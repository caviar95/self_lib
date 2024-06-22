/*
 * @Author: Caviar
 * @Date: 2024-06-22 22:24:16
 * @LastEditors: Caviar
 * @LastEditTime: 2024-06-23 00:36:52
 * @Description: bshoshany thread_pool based on c++17
 */

#pragma once

#include <chrono> // std::chrono
#include <future> // std::promise/std::shared_future
#include <initializer_list> // std::initializer_list
#include <iostream> // std::cout/std::endl/std::flush/std::ostream
#include <memory> // std::make_unique/std::unique_ptr
#include <mutex> // std::mutex/std::scoped_lock
#include <utility> // std::forward

#include "../type.h"
#include "../macro.h"

namespace BS {

constexpr uint32_t THREAD_POOL_UTILS_VERSION_MAJOR{4};
constexpr uint32_t THREAD_POOL_UTILS_VERSION_MINOR{1};
constexpr uint32_t THREAD_POOL_UTILS_VERSION_PATCH{0};

// a utility class to allow simple signalling between threads
class [[nodiscard]] signaller {
public:
    signaller() : future_(promise_.get_future()) {}

    DISALLOW_COPY_AND_ASSIGN(signaller);
    ALLOW_MOVE(signaller);

    // inform any waiting threads that the signaller is ready
    void ready() {
        promise_.set_value();
    }

    // wait until the signaller is ready
    void wait() {
        future_.wait();
    }

private:
    // a promise used to set the state of the signaller
    std::promise<void> promise_{};

    // a future used to wait for the signaller
    std::shared_future<void> future_{};
};

// a utility class to synchronize printing to an output stream by different threads
class [[nodiscard]] synced_stream {
public:
    DISALLOW_COPY_AND_MOVE(synced_stream);
    
    // print any number of items into the output stream. ensures that no other threads print to this stream simulateneous,
    // as long as they all exclusively use the same 'synced_stream' object to print.
    template <typename... T>
    void println(T&&... items) {
        print(std::forward<T>(items)..., '\n');
    }

    // a stream manipulator to pass to a 'synced_stream'(an explicit cast of 'std::endl'). prints a newline character to the stream,
    // and then flushed it. should only be used if flushing is desired, otherwise a newline character shoule be used instead.
    inline static std::ostream& (&endl)(std::ostream&) = static_cast<std::ostream& (&)(std::ostream&)>(std::endl);

    // a stream manipulator to pass to a 'synced_stream' (an explicit cast of 'std::flush'). used to flush the stream.
    inline static std::ostream& (&flush)(std::ostream&) = static_cast<std::ostream& (&)(std::ostream&)>(std::flush);

private:
    // the output stream to print to
    std::ostream &out_stream_;

    // a mutex to synchronize printing
    mutable std::mutex stream_mutex_{};
};

// a utility class to measure execution time for benchmarking purposes.
class [[nodiscard]] timer {
public:
    timer() = default;

    // get the number of miiliseconds that have elapsed since the object was constructed or since 'start()' was last called,
    // but keep the timer ticking.
    [[nodiscard]] std::chrono::milliseconds::rep current_ms() const {
        return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_)).count();
    }

    // start/restart measuring time. note that the timer starts ticking as soon as the object is created, so this is only
    // necessary if we want to restart the clock later.
    void start() {
        start_time_ = std::chrono::steady_clock::now();
    }

    // stop measuring time and store the elapsed time since the object was constructed or since 'start()' was last called.
    void stop() {
        elapsed_time_ = std::chrono::steady_clock::now() - start_time_;
    }

    // get the number of milliseconds stored when 'stop()' was last called.
    [[nodiscard]] std::chrono::milliseconds::rep ms() const {
        return (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time_)).count();
    }

private:
    // the time point when measuring started.
    std::chrono::time_point<std::chrono::steady_clock> start_time_ = std::chrono::steady_clock::now();

    // the duration that has elapsed between 'start()' and 'stop()'
    std::chrono::duration<double> elapsed_time_ = std::chrono::duration<double>::zero();
};

}
