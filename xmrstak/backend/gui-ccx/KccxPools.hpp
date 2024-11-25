#pragma once
#include <string>
#include <vector>

struct KPool {
    std::string name;
    std::string url;
    int port;
    bool ssl;
};

std::vector<KPool> loadKnownPools(const std::string& jsonPath);
