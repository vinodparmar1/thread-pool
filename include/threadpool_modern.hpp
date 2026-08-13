#pragma once

#include <thread>
#include <concepts>
#include <future>
#include <memory>
#include <functional>
#include <queue>
#include <condition_variable>

namespace threadpool {
constexpr size_t THREADPOOL_MIN_THREADS = 2;

class ThreadpoolM {
public:
    ThreadpoolM(size_t totalThread = THREADPOOL_MIN_THREADS) : totalThreads_(totalThread) {
        // hardware_concurrency can return 0 also, check it and if so keep the min of threads
        auto hardware_allowed_threads = std::thread::hardware_concurrency();
        if(hardware_allowed_threads == 0) {
            totalThreads_ = THREADPOOL_MIN_THREADS;
        }
        else
            // create the threads max upto allowed by hardware concurrency
            totalThreads_ = std::min(totalThread , static_cast<size_t>(hardware_allowed_threads));
        
        for(size_t i = 0; i <totalThreads_; ++i) {
            // emplace the thread into thread vector
            threads.emplace_back([this](std::stop_token st) {
                // Worder thread: run undefinitly and keep taking task and finish it
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(m);
                        /* unblock the thread using cv when either the task queue have some task or 
                        pool is stopped */
                        cv.wait(lock, st, [this]() { return !taskq.empty(); });
                        
                        /* */
                        if(st.stop_requested() && taskq.empty()) return; // drained + stopping
                        if(taskq.empty()) continue; // queue is empty but stop is not requested
                        task = std::move(taskq.front());
                        taskq.pop();
                    }
                    // execute the task
                    task(); 
                }
            });
        }
    }

    // take callable F and variadic template arguments to accept a task of any signature
    template< class F, class... Args>
    requires std::invocable<F, Args...>  /* concept to strict checking */
    /* submit take the callable and it's arguments. Derive the return type using modern C++ */
    auto submit(F&& f, Args&&... args)-> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;
        // Create a packaged task to wrap the callable and it's argument
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            /* forward the callable and it's argument so the exact supplied type goes as is */
            [f= std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable{
                // run the task
                return std::invoke(f, args...);
            }
        );

        /* get the future from packaged_task and give it to caller for getting return values from task supplied
        at later time */
        auto fut = task->get_future();
        {
            // lock_guard, lighter than unique_lock here
            std::lock_guard lock(m);
            taskq.push([task]() { (*task)(); });
        }
        /* Notify 1 thread from the waiting queue of the cv */
        cv.notify_one();
        return fut;
    }

    ~ThreadpoolM() = default;
    ThreadpoolM(const ThreadpoolM&) = delete;
    ThreadpoolM& operator=(const ThreadpoolM&) = delete;

private:
    size_t totalThreads_;
    std::queue<std::function<void()>> taskq;
    std::mutex m;
    std::condition_variable_any cv; // _any, for stop aware wait
    std::vector<std::jthread> threads; // using jthread, automatically joins on destruction
};

}
