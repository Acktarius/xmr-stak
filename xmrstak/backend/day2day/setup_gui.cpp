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
#include "../../jconf.hpp"
#include "../../params.hpp"


enum {
    ID_START_BUTTON = wxID_HIGHEST + 1
};

std::tuple<bool, std::string, std::string, std::string> readMiningConfig()
{
    std::cout << "DEBUG: Starting readMiningConfig()" << std::endl;
       
    // Get pool address and wallet directly from jconf instance
    const std::string& poolAddr = jconf::GetDefaultPool(xmrstak::params::inst().poolURL.c_str());
    const std::string& walletAddr = jconf::GetDefaultPool(xmrstak::params::inst().poolUsername.c_str());
    const std::string& currency = jconf::GetDefaultPool(xmrstak::params::inst().currency.c_str());

    return std::make_tuple(!poolAddr.empty(), poolAddr, walletAddr, currency);
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
    std::cout << "DEBUG: Starting OnInit()" << std::endl;
    
    if (!wxApp::OnInit())
        return false;

    std::cout << "DEBUG: About to call readMiningConfig()" << std::endl;
    
    // Create an instance of ReadPoolConfig
    auto [fileReachable, pool, wallet, currency] = readMiningConfig();
    
    std::cout << "DEBUG: readMiningConfig() returned fileReachable=" << fileReachable << std::endl;
    
    if (fileReachable == false)
    {
        wxMessageBox("The pools.txt file is missing or not in the /bin folder.\n"
                     "Please create this file by running xmr-stak for initial setup.",
                     "Configuration Missing",
                     wxOK | wxICON_ERROR);
        return false;
    }
     {
        wxMessageBox("The pools.txt file has been detect,"
                     + pool + " " + wallet + " " + currency,
                     "Configuration",
                     wxOK | wxICON_INFORMATION);
    }
    MiningConfigFrame* frame = new MiningConfigFrame(nullptr, wxID_ANY, "XMR-Stak Day2Day Mining", wxDefaultPosition, wxSize(400, 300));
    frame->SetPoolInfo(pool, wallet);  // Assuming you have this method in MiningConfigFrame
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(GUIApp);
