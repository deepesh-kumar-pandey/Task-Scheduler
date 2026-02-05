#include "../include/TaskScheduler.h"
#include "../include/config.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <unistd.h>

using namespace std;
using namespace std::chrono;

int main() {
    // 1. Security check using your Config implementation
    string secret_key = Config::get_encryption_key();

    // 2. Hardware-Aware Thread Configuration
    // Detect cores and subtract 1 to leave room for the Monitor thread/OS
    unsigned int hardware_cores = std::thread::hardware_concurrency();
    int worker_count = (hardware_cores > 1) ? (hardware_cores - 1) : 1;

    cout << "--- [BACKEND ENGINE INITIALIZED] ---" << endl;
    cout << "[SYS] Cores detected: " << hardware_cores << " | Workers: " << worker_count << endl;

    // 3. Initialize and Start the Scheduler
    TaskScheduler scheduler(worker_count);
    scheduler.Start();

    string taskName;
    int delaySeconds;

    bool is_interactive = isatty(fileno(stdin));

    if (is_interactive) {
        cout << "\nTask Engine is live. Enter details (or type 'exit' to quit):" << endl;
    }

    while(true) {
        if (is_interactive) cout << "Task name: \n";
        if (!(getline(cin, taskName))) break;

        if(taskName == "exit" || taskName == "quit") break;

        if (is_interactive) cout << "Delay in seconds: \n";
        if(!(cin >> delaySeconds)) {
            cout << "Invalid input.Please enter a number. \n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        Task t;
        t.name = taskName;
        t.payload = "{ \"secret_key\" : \"" + secret_key + "\" }";
        t.execution_time = system_clock::now() + seconds(delaySeconds);
        t.status = TaskStatus::PENDING;
        t.attempts = 0;
        t.max_attempts = 3;

        t.task_logic = [taskName, delaySeconds]() {
            // cout << "\n[EXECUTION] Processing task: " << taskName 
            //      << " | Thread ID: " << this_thread::get_id() << "\n";
            std::this_thread::sleep_for(seconds(delaySeconds));
            // cout << "   [COMPLETED] Task: " << taskName << "\n";
        };

        scheduler.Schedule_task(std::move(t));
        if (is_interactive) {
            cout << "[SCHEDULED] Task " << taskName << " set to run in " << delaySeconds << " seconds.\n";
        }
    }

    scheduler.Stop();
    cout << "\n[SYSTEM] Engine stopped.\n";
    return 0;
}