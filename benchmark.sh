#!/bin/bash

# Configuration
TOTAL_TASKS=300000
INPUT_FILE="benchmark_input.txt"
EXECUTABLE="./main"
LOG_FILE="benchmark_log.txt"

# Ensure executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Compiling..."
    g++ -o main src/main.cpp src/config.cpp src/TaskScheduler.cpp -I include -lpthread
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

echo "Generating $TOTAL_TASKS tasks..."
seq 1 $TOTAL_TASKS | awk '{print "Task_" $1 "\n0"}' > "$INPUT_FILE"
echo "exit" >> "$INPUT_FILE"

echo "Starting benchmark..."
echo "This may take a while depending on your CPU."

start_time=$(date +%s%N)
$EXECUTABLE < "$INPUT_FILE" > /dev/null
end_time=$(date +%s%N)

duration_ms=$(( (end_time - start_time) / 1000000 ))
duration_sec=$(( duration_ms / 1000 ))

echo "Benchmark completed."
echo "Total Tasks Scheduled: $TOTAL_TASKS"
echo "Total Time: ${duration_sec}s (${duration_ms}ms)"

if [ $duration_ms -gt 0 ]; then
    throughput=$(( (TOTAL_TASKS * 1000) / duration_ms ))
    avg_latency_us=$(( (duration_ms * 1000) / TOTAL_TASKS ))
    echo "Throughput: $throughput tasks/sec"
    echo "Avg Latency: ${avg_latency_us} us/task"
else
    echo "Execution too fast to measure meaningful latency."
fi

rm "$INPUT_FILE"
