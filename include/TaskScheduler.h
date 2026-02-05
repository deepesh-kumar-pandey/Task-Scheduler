#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <chrono>
#include <atomic>
#include <memory>

/**
 * @brief Represents the current lifecycle stage of a scheduled task.
 */
enum class TaskStatus {
    PENDING,      // Task is created but not yet in the queue
    IN_PROGRESS,  // Task is currently being handled by a worker
    COMPLETED,    // Task finished successfully
    FAILED,       // Task failed after all retries
    PROCESSING    // Task has been picked up from the queue
};

/**
 * @brief The data structure representing a unit of work.
 */
struct Task {
    std::string id;           // Unique identifier (Primary Key for DB)
    std::string name;         // Human-readable name
    std::string payload;      // JSON or raw data required for execution
    TaskStatus status;        // Current state
    int attempts;             // Current retry count
    int max_attempts;         // Maximum allowed retries before FAILED status
    
    // The exact wall-clock time when this task should be executed
    std::chrono::system_clock::time_point execution_time; 
    
    // The actual logic/function to run (Command Pattern)
    std::function<void()> task_logic; 

    /**
     * @brief Custom Comparator for Min-Heap.
     * By using '>', we tell the priority_queue that a task with a LATER 
     * time is "greater" (lower priority), keeping the EARLIEST task at the top.
     */
    struct CompareTimestamp {
        bool operator()(const Task& t1, const Task& t2) {
            return t1.execution_time > t2.execution_time;
        }
    };

    // Move semantics support
    Task() = default;
    Task(Task&&) = default;
    Task& operator=(Task&&) = default;
    Task(const Task&) = default;
    Task& operator=(const Task&) = default;
};

class TaskScheduler {
    private:
        // Structure representing a single shard (Queue + Lock + CV)
        struct WorkerQueue {
            std::mutex mutex_lock;
            std::priority_queue<Task, std::vector<Task>, Task::CompareTimestamp> task_queue;
            std::condition_variable cv;
        };

        // Tasks are distributed among these shards
        // Using unique_ptr because mutexes are not copyable/movable
        std::vector<std::unique_ptr<WorkerQueue>> shards;
        
        // Thread pool
        std::vector<std::thread> worker_threads;
        
        // Atomic flag for shutdown
        bool stop;
        
        // Internal Worker loop: Manages its own designated shard
        void Worker_thread(int shard_id);

        // Atomic counter for Round-Robin scheduling
        std::atomic<size_t> task_counter;

        int num_workers;

    public:
        /**
         * @brief Initialize the scheduler with specific thread counts.
         */
        TaskScheduler(int workers); // Monitors arg removed as not needed
        
        ~TaskScheduler();

        TaskScheduler(const TaskScheduler& other) = delete;
        TaskScheduler& operator=(const TaskScheduler& other) = delete;

        void Schedule_task(Task&& task);
        void Schedule_task(const Task& task);
        void Start();
        void Stop();
}; // End of class

#endif // TASK_SCHEDULER_H