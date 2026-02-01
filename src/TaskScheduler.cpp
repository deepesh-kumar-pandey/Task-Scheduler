#include "TaskScheduler.h"
#include <iostream>

// Constructor: Initializes threads and the stop flag
TaskScheduler::TaskScheduler(int workers, int monitors) 
    : stop(false), num_workers(workers), num_monitors(monitors) {}

// Destructor: Ensures we don't leave "Zombie Threads"
TaskScheduler::~TaskScheduler() {
    Stop();
}

void TaskScheduler::Start() {
    // 1. Launch Monitor threads (The "Watchers")
    for (int i = 0; i < num_monitors; ++i) {
        monitor_threads.emplace_back(&TaskScheduler::Monitor_queue, this);
    }

    // 2. Launch Worker threads (The "Doers")
    for (int i = 0; i < num_workers; ++i) {
        worker_threads.emplace_back(&TaskScheduler::Worker_thread, this);
    }
}

void TaskScheduler::Schedule_task(const Task& task) {
    {
        std::lock_guard<std::mutex> lock(mutex_lock);
        task_queue.push(task); 
    }
    // Signal the monitor that a new task is in the heap
    cv_monitor.notify_one();
}

void TaskScheduler::Monitor_queue() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_lock);

        // Wait until there is a task OR the system stops
        cv_monitor.wait(lock, [this] { 
            return !task_queue.empty() || stop; 
        });

        if (stop) return;

        auto now = std::chrono::system_clock::now();
        const Task& topTask = task_queue.top();

        if (topTask.execution_time <= now) {
            // Task is due! Wake up a worker
            cv_worker.notify_one();
            // In a full implementation, we'd move this to a "Ready" queue
            // For now, let's let the worker handle it directly
            lock.unlock(); 
        } else {
            // Task isn't due yet. Sleep until it is.
            cv_monitor.wait_until(lock, topTask.execution_time);
        }
    }
}

void TaskScheduler::Worker_thread() {
    while (true) {
        Task current_task;
        {
            std::unique_lock<std::mutex> lock(mutex_lock);
            
            // Wait for a task that is actually ready to run
            cv_worker.wait(lock, [this] {
                return (!task_queue.empty() && 
                        task_queue.top().execution_time <= std::chrono::system_clock::now()) 
                        || stop;
            });

            if (stop && task_queue.empty()) return;

            // Grab the task from our Min-Heap
            current_task = std::move(const_cast<Task&>(task_queue.top()));
            task_queue.pop();
        }

        // Execute the logic stored in std::function
        if (current_task.task_logic) {
            try {
                current_task.task_logic();
            } catch (const std::exception& e) {
                std::cerr << "Task failed: " << e.what() << std::endl;
            }
        }
    }
}

void TaskScheduler::Stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_lock);
        stop = true;
    }
    // Wake up everyone so they can see the 'stop' flag and exit
    cv_monitor.notify_all();
    cv_worker.notify_all();

    for (auto& t : monitor_threads) if (t.joinable()) t.join();
    for (auto& t : worker_threads) if (t.joinable()) t.join();
}