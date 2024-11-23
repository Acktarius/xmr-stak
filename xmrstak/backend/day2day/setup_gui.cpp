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

std::tuple<bool, std::string, std::string, std::string> readMiningConfig()
{
    try {
        std::ifstream file("pools.txt", std::ifstream::binary);
        if (!file.is_open()) {
            return std::make_tuple(false, "", "", "");
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
            return std::make_tuple(false, "", "", "");
        }

        if (!root.isMember("pool_list") || !root["pool_list"].isArray()) {
            return std::make_tuple(false, "", "", "");
        }

        const Json::Value& poolList = root["pool_list"];
        if (poolList.empty()) {
            return std::make_tuple(false, "", "", "");
        }

        std::string poolAddress = poolList[0]["pool_address"].asString();
        std::string walletAddress = poolList[0]["wallet_address"].asString();
        std::string currency = root["currency"].asString();

        return std::make_tuple(true, poolAddress, walletAddress, currency);
    }
    catch (const Json::Exception& e) {
        return std::make_tuple(false, "", "", "");
    }
    catch (const std::exception& e) {
        return std::make_tuple(false, "", "", "");
    }
    catch (...) {
        return std::make_tuple(false, "", "", "");
    }
}

// Implement the member functions of MiningConfigFrame
MiningConfigFrame::MiningConfigFrame(wxWindow* parent, wxWindowID id, const wxString& title, 
                                     const wxPoint& pos, const wxSize& size)
    : wxFrame(parent, id, title, pos, size)
{
    // Constructor implementation
    // ...
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

    // Create an instance of ReadPoolConfig
    auto [fileReachable, pool, wallet, currency] = readMiningConfig();
    
    if (fileReachable == false)
    {
        wxMessageBox("The pools.txt file is missing or not in the /bin folder.\n"
                     "Please create this file by running xmr-stak for initial setup.",
                     "Configuration Missing",
                     wxOK | wxICON_ERROR);
        return false;
    }
      if (currency != "conceal")
        {
        wxMessageBox("this user interface is developped for Conceal users.\n",
                     "Please consider mining CCX",
                     wxOK | wxICON_ERROR);
        return false;
        }
        {
            wxMessageBox("The pools.txt file has been detected\n"
                        "Pool: " + wxString(pool) + "\n"
                        "Wallet: " + wxString(wallet) + "\n"
                        "Mining: " + wxString(currency),
                        "Configuration found",
                        wxOK | wxICON_INFORMATION);
        }
    MiningConfigFrame* frame = new MiningConfigFrame(nullptr, wxID_ANY, "XMR-Stak Day2Day Mining", wxDefaultPosition, wxSize(600, 400));
    frame->SetPoolInfo(pool, wallet);  // Assuming you have this method in MiningConfigFrame
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(GUIApp);
