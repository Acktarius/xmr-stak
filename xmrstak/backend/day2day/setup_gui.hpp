#pragma once

#include <wx/wx.h>
#include <tuple>
#include <string>

class MiningConfigFrame : public wxFrame
{
public:
    MiningConfigFrame(wxWindow* parent, wxWindowID id, const wxString& title, 
                    const wxPoint& pos = wxDefaultPosition, 
                    const wxSize& size = wxDefaultSize);

    void SetPoolInfo(const std::string& pool, const std::string& wallet) {
        m_pool = pool;
        m_wallet = wallet;
        UpdateDisplay();
    }

private:
    void OnStart(wxCommandEvent& event);
    void UpdateDisplay();
    std::string m_pool;
    std::string m_wallet;
    wxStaticText* m_poolText;
    wxStaticText* m_walletText;
};

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

class GUIApp : public wxApp
{
public:
    virtual bool OnInit() override;

private:
    // You might want to add any private members here if needed
};
