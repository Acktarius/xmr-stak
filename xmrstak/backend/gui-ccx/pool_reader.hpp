#ifndef XMR_STAK_POOL_READER_H
#define XMR_STAK_POOL_READER_H

#include <string>

namespace xmrstak {
namespace config {

class PoolConfig
{
public:
    PoolConfig() : m_fileReachable(false) {}
    PoolConfig(bool fileReachable, const std::string& poolAddress, 
               const std::string& walletAddress, const std::string& currency)
        : m_fileReachable(fileReachable)
        , m_poolAddress(poolAddress)
        , m_walletAddress(walletAddress)
        , m_currency(currency) {}

    bool isValid() const { return m_fileReachable; }
    const std::string& getPoolAddress() const { return m_poolAddress; }
    const std::string& getWalletAddress() const { return m_walletAddress; }
    const std::string& getCurrency() const { return m_currency; }

private:
    bool m_fileReachable;
    std::string m_poolAddress;
    std::string m_walletAddress;
    std::string m_currency;
};

PoolConfig readMiningConfig();

} // namespace config
} // namespace xmrstak

#endif // XMR_STAK_POOL_READER_H
