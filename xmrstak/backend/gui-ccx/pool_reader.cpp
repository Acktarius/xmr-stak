#include "pool_reader.hpp"
#include <fstream>
#include <sstream>
#include <jsoncpp/json/json.h>

namespace xmrstak {
namespace config {

PoolConfig readMiningConfig()
{
    try {
        std::ifstream file("pools.txt", std::ifstream::binary);
        if (!file.is_open()) {
            return PoolConfig();
        }

        // Read file content into string
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Remove C-style comments
        size_t start, end;
        while ((start = content.find("/*")) != std::string::npos) {
            if ((end = content.find("*/", start)) != std::string::npos) {
                content.erase(start, end - start + 2);
            }
        }

        // Remove single-line comments
        while ((start = content.find("//")) != std::string::npos) {
            if ((end = content.find('\n', start)) != std::string::npos) {
                content.erase(start, end - start);
            } else {
                content.erase(start);
            }
        }

        // Add enclosing braces if not present
        if (content.find_first_not_of(" \t\n\r") != '{') {
            content = "{" + content + "}";
        }

        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        std::string errs;
        std::istringstream jsonStream(content);

        if (!Json::parseFromStream(builder, jsonStream, &root, &errs)) {
            return PoolConfig();
        }

        if (!root.isMember("pool_list") || !root["pool_list"].isArray()) {
            return PoolConfig();
        }

        const Json::Value& poolList = root["pool_list"];
        if (poolList.empty()) {
            return PoolConfig();
        }

        return PoolConfig(
            true,
            poolList[0]["pool_address"].asString(),
            poolList[0]["wallet_address"].asString(),
            poolList[0]["use_tls"].asBool(),
            root["currency"].asString()
        );
    }
    catch (...) {
        return PoolConfig();
    }
}

} // namespace config
} // namespace xmrstak
