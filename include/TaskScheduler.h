#define TaskScheduler_h
#ifndef TaskScheduler_h

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <chrono>

enum class TaskStatus {
    PENDING, IN_PROGRESS, COMPLETED, FAILED, PROCESSING
};

struct Task {
    std::string id;
    std::string name;
    std::string payload;
    TaskStatus status;
    int attempts;
    int max_attempts;
    std::chrono::system_clock::time_point execution_time;
    std::function<void()> task;

    bool operator<(const Task& other) const {
        return execution_time < other.execution_time;
    }
};

class TaskScheduler {
    private:
        std::mutex mutex_lock;
        std::priority_queue<Task> task_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_monitor;
        std::vector<std::thread> worker_threads;
        std::vector<std::thread> monitor_threads;
        bool stop;

        void Monitor_queue();
        void Worker_thread();
}