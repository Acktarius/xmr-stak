#include "setup_gui.hpp"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>
#include <wx/string.h>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <sstream>


enum {
    ID_START_BUTTON = wxID_HIGHEST + 1
};

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
            root["currency"].asString()
        );
    }
    catch (...) {
        return PoolConfig();
    }
}

// Implement the member functions of MiningConfigFrame
MiningConfigFrame::MiningConfigFrame(wxWindow* parent, wxWindowID id, const wxString& title, 
                                     const wxPoint& pos, const wxSize& size)
    : wxFrame(parent, id, title, pos, size)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Store pointers to text controls
    m_poolText = new wxStaticText(this, wxID_ANY, "Mining Pool: ");
    m_walletText = new wxStaticText(this, wxID_ANY, "Wallet: ");
    
    mainSizer->Add(m_poolText, 0, wxALL, 10);
    mainSizer->Add(m_walletText, 0, wxALL, 10);
    
    SetSizer(mainSizer);
}

void MiningConfigFrame::UpdateDisplay()
{
    m_poolText->SetLabel("Mining Pool: " + wxString(m_pool));
    m_walletText->SetLabel("Wallet: " + wxString(m_wallet));
    Layout(); // Ensure the frame updates properly
}

void MiningConfigFrame::OnStart(wxCommandEvent& event)
{
    // Your OnStart implementation here
}


// Implement the member functions of MyApp
bool GUIApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    PoolConfig config = readMiningConfig();
    
    if (!config.isValid())
    {
        wxMessageBox("The pools.txt file is missing or not in the /bin folder.\n"
                     "Please create this file by running xmr-stak for initial setup.",
                     "Configuration Missing",
                     wxOK | wxICON_ERROR);
        return false;
    }

    if (config.getCurrency() != "conceal")
    {
        wxMessageBox("this user interface is developped for Conceal users.\n",
                     "Please consider mining CCX",
                     wxOK | wxICON_ERROR);
        return false;
    }
/*
    wxMessageBox("The pools.txt file has been detected\n"
                "Pool: " + wxString(config.getPoolAddress()) + "\n"
                "Wallet: " + wxString(config.getWalletAddress()) + "\n"
                "Mining: " + wxString(config.getCurrency()),
                "Configuration found",
                wxOK | wxICON_INFORMATION);
*/
    MiningConfigFrame* frame = new MiningConfigFrame(nullptr, wxID_ANY, 
        "XMR-Stak-CCX", wxDefaultPosition, wxSize(800, 400));
    frame->SetPoolInfo(config.getPoolAddress(), config.getWalletAddress());
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(GUIApp);
