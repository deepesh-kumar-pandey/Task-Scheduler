# Task Scheduler

A robust, multi-threaded task scheduling system designed for efficient task management and execution in C++. The scheduler utilizes a priority queue to handle tasks based on their scheduled execution time, ensuring timely processing through an optimized thread pool.

## Tech Stack

- **Language**: C++ (C++11+)
- **Concurrency**: standard threads (`std::thread`), mutexes (`std::mutex`), and condition variables (`std::condition_variable`)
- **Data Structures**: Priority Queue (`std::priority_queue`), Vector (`std::vector`)
- **Functional Programming**: Lambda functions and `std::function` for task definitions
- **Time Management**: `std::chrono` for precise execution scheduling
- **Configuration**: Environment-based configuration management

## How to Run

### Prerequisites

- GCC/G++ Compiler
- Linux/Unix environment (recommended)

### 1. Compile the Project

```bash
g++ -o main src/main.cpp src/config.cpp src/TaskScheduler.cpp -I include -lpthread
```

### 2. Configure Environment

Create a `.env` file or export the key manually.

```bash
# Option 1: Create .env
echo "MONITOR_KEY=YourSecretKey" > .env

# Option 2: Export directly
export MONITOR_KEY="YourSecretKey"
```

_Note: If no key is provided, the application will automatically generate a random one for the session._

### 3. Run the Application

```bash
export $(grep -v '^#' .env | xargs) && ./main
```

## Example Output

Here is an example of an interactive session:

```text
--- [BACKEND ENGINE INITIALIZED] ---
[SYS] Cores detected: 12 | Workers: 11
Task Engine is live. Enter details (or type 'exit' to quit):

Task name:
DataCleanup
Delay in seconds:
2
[SCHEDULED] Task DataCleanup set to run in 2 seconds.

[EXECUTION] Processing task: DataCleanup | Thread ID: 140239281293056
   [COMPLETED] Task: DataCleanup
```

## Benchmarking

We have included a script `benchmark.sh` to load test the scheduler with **300,000 tasks**.

### Running the Benchmark

You can run the benchmark script yourself to replicate the performance test and verify the throughput/latency on your own hardware.

```bash
chmod +x benchmark.sh
./benchmark.sh
```

### Expected Results (Sample)

```text
Generating 300000 tasks...
Starting benchmark...
This may take a while depending on your CPU.
Benchmark completed.
Total Tasks Scheduled: 300000
Total Time: 3s (3450ms)
Throughput: 87000 tasks/sec
Avg Latency: 11 us/task
Input file cleaned up.
```

_Performance depends on system hardware. The above result was observed on a high-concurrency system with optimized move semantics and batching._
