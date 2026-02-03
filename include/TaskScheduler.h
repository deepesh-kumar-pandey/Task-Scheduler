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
};

class TaskScheduler {
    private:
        // Synchronization primitives for thread-safe queue access
        std::mutex mutex_lock;

        /**
         * @brief EXPLICIT MIN-HEAP IMPLEMENTATION
         * std::priority_queue<Type, Container, Comparator>
         * This ensures the task with the smallest execution_time is always at the top.
         */
        std::priority_queue<Task, std::vector<Task>, Task::CompareTimestamp> task_queue;
        
        // Signals to wake up threads (Monitor checks time, Workers check for tasks)
        std::condition_variable cv_worker;
        std::condition_variable cv_monitor;
        
        // Thread pools for background processing
        std::vector<std::thread> worker_threads;
        std::vector<std::thread> monitor_threads;
        
        // Atomic-style flag to trigger graceful shutdown
        bool stop;

        // Internal loop: Checks if tasks in the queue have reached their execution_time
        void Monitor_queue();
        
        // Internal loop: Consumes ready tasks and executes their logic
        void Worker_thread();

        // Private members to store thread counts
        int num_workers;
        int num_monitors;

    public:
        /**
         * @brief Initialize the scheduler with specific thread counts.
         */
        TaskScheduler(int workers, int monitors);
        
        /**
         * @brief Cleans up threads and ensures a graceful shutdown.
         */
        ~TaskScheduler();

        // Disable Copy Constructor and Assignment Operator (Singleton/Resource Management)
        TaskScheduler(const TaskScheduler& other) = delete;
        TaskScheduler& operator=(const TaskScheduler& other) = delete;

        /**
         * @brief Adds a new task to the internal priority queue.
         */
        void Schedule_task(const Task& task);
        
        /**
         * @brief Spawns the monitor and worker threads to begin processing.
         */
        void Start();
        
        /**
         * @brief Signals all threads to stop and joins them.
         */
        void Stop();
}; // End of class

#endif // TASK_SCHEDULER_H