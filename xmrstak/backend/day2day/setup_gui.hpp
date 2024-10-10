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
        // Update GUI elements if necessary
    }

private:
    void OnStart(wxCommandEvent& event);
    std::string m_pool;
    std::string m_wallet;
    // Other private members and methods
};

class PoolConfig
{
public:
    PoolConfig(bool fileReachable, const std::string& poolAddress, const std::string& walletAddress);

    std::tuple<bool, std::string, std::string> read();

private:
    bool m_fileReachable;
    std::string m_poolAddress;
    std::string m_walletAddress;
};

class GUIApp : public wxApp
{
public:
    virtual bool OnInit() override;

private:
    // You might want to add any private members here if needed
};
