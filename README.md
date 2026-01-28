# Task Scheduler

A robust, multi-threaded task scheduling system designed for efficient task management and execution in C++. The scheduler utilizes a priority queue to handle tasks based on their scheduled execution time, ensuring timely processing through an optimized thread pool.

## Tech Stack

- **Language**: C++ (C++11+)
- **Concurrency**: standard threads (`std::thread`), mutexes (`std::mutex`), and condition variables (`std::condition_variable`)
- **Data Structures**: Priority Queue (`std::priority_queue`), Vector (`std::vector`)
- **Functional Programming**: Lambda functions and `std::function` for task definitions
- **Time Management**: `std::chrono` for precise execution scheduling
- **Configuration**: Environment-based configuration management