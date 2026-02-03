#include "../include/config.h"
#include <iostream>
#include <cstdlib>

#include <random>
#include <sstream>
#include <iomanip>

std::string Config::get_encryption_key() {
    const char* env_key = std::getenv("MONITOR_KEY");
    
    if (env_key != nullptr) {
        return std::string(env_key);
    }

    std::cout << "[Config] Warning: MONITOR_KEY not set. Generating a temporary key for this session..." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    std::string new_key = ss.str();
    std::cout << "[Config] Generated Key: " << new_key << std::endl;
    
    return new_key;
}