#include "../include/TaskScheduler.h"
#include <iostream>

TaskScheduler::TaskScheduler(int workers) 
    : stop(false), num_workers(workers), task_counter(0) {
    
    // Initialize Shards
    for(int i = 0; i < num_workers; ++i) {
        shards.push_back(std::unique_ptr<WorkerQueue>(new WorkerQueue()));
    }
}

TaskScheduler::~TaskScheduler() {
    Stop();
}

void TaskScheduler::Start() {
    // Launch Worker threads, each assigned to a specific shard
    for (int i = 0; i < num_workers; ++i) {
        worker_threads.emplace_back(&TaskScheduler::Worker_thread, this, i);
    }
}

void TaskScheduler::Schedule_task(Task&& task) {
    size_t current_id = task_counter.fetch_add(1);
    int shard_id = current_id % num_workers;

    {
        std::lock_guard<std::mutex> lock(shards[shard_id]->mutex_lock);
        shards[shard_id]->task_queue.push(std::move(task));
    }
    shards[shard_id]->cv.notify_one();
}

void TaskScheduler::Schedule_task(const Task& task) {
    Task copy = task;
    Schedule_task(std::move(copy));
}

void TaskScheduler::Worker_thread(int shard_id) {
    WorkerQueue* my_shard = shards[shard_id].get();
    std::vector<Task> ready_tasks;
    ready_tasks.reserve(1000);

    while (true) {
        ready_tasks.clear();
        {
            std::unique_lock<std::mutex> lock(my_shard->mutex_lock);
            
            my_shard->cv.wait(lock, [this, my_shard] { 
                return stop || !my_shard->task_queue.empty(); 
            });

            if (stop && my_shard->task_queue.empty()) return;

            while (!my_shard->task_queue.empty()) {
                const Task& topTask = my_shard->task_queue.top();
                auto now = std::chrono::system_clock::now();

                if (topTask.execution_time > now) {
                    if (ready_tasks.empty()) {
                        auto wake_time = topTask.execution_time;
                        my_shard->cv.wait_until(lock, wake_time);
                        break; 
                    } else {
                        break; 
                    }
                }

                ready_tasks.push_back(std::move(const_cast<Task&>(my_shard->task_queue.top())));
                my_shard->task_queue.pop();
                
                if (ready_tasks.size() >= 1000) break;
            }
        }

        for (auto& task : ready_tasks) {
            if (task.task_logic) {
                try {
                    task.task_logic();
                } catch (const std::exception& e) {
                    std::cerr << "Task failed: " << e.what() << std::endl;
                }
            }
        }
    }
}

void TaskScheduler::Stop() {
    stop = true;
    
    // Wake up all workers so they can exit
    for(auto& shard : shards) {
        std::lock_guard<std::mutex> lock(shard->mutex_lock);
        shard->cv.notify_all();
    }

    for (auto& t : worker_threads) {
        if (t.joinable()) t.join();
    }
}