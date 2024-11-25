#include "KccxPools.hpp"
#include <jsoncpp/json/json.h>
#include <fstream>
#include <iostream>

std::vector<KPool> loadKnownPools(const std::string& jsonPath) {
    std::vector<KPool> Kpools;
    std::ifstream file(jsonPath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << jsonPath << std::endl;
        return Kpools;
    }

    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(file, root)) {
        std::cerr << "Failed to parse JSON: " << reader.getFormattedErrorMessages() << std::endl;
        return Kpools;
    }

    // Try both "pools" and "knownPools" keys
    const Json::Value& poolArray = root.isMember("knownPools") ? root["knownPools"] : 
                                 (root.isMember("pools") ? root["pools"] : Json::Value::null);

    if (poolArray.isNull()) {
        std::cerr << "No 'pools' or 'knownPools' array found in JSON" << std::endl;
        return Kpools;
    }

    if (!poolArray.isArray()) {
        std::cerr << "The 'pools' value is not an array" << std::endl;
        return Kpools;
    }

    for (const auto& pool : poolArray) {
        if (!pool.isMember("name") || !pool.isMember("url") || !pool.isMember("port")) {
            std::cerr << "Pool entry missing required fields" << std::endl;
            continue;
        }

        try {
            KPool kpool;
            kpool.name = pool["name"].asString();
            kpool.url = pool["url"].asString();
            kpool.port = pool["port"].asInt();
            kpool.ssl = pool.get("ssl", false).asBool(); // Optional field with default value
            
            Kpools.push_back(kpool);
            
            // Debug output
            std::cout << "Loaded pool: " << kpool.name 
                      << " (" << kpool.url << ":" << kpool.port 
                      << ", SSL: " << (kpool.ssl ? "yes" : "no") << ")" << std::endl;
        }
        catch (const Json::Exception& e) {
            std::cerr << "Error parsing pool entry: " << e.what() << std::endl;
        }
    }

    std::cout << "Total pools loaded: " << Kpools.size() << std::endl;
    return Kpools;
}