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

enum {
    ID_START_BUTTON = wxID_HIGHEST + 1
};
//readMiningConfig
std::tuple<bool, std::string, std::string> readMiningConfig()
{
    wxString poolsFilePath = wxFileName::GetCwd() + wxFileName::GetPathSeparator() + "pools.txt";
    bool poolsFileExists = wxFileExists(poolsFilePath);

    if (!poolsFileExists)
    {
        return std::make_tuple(false, std::string(), std::string());
    }

    wxTextFile file(poolsFilePath);
    if (!file.Open())
    {
        return std::make_tuple(false, std::string(), std::string());
    }

    file.Close();

    wxString poolAddress, walletAddress;
    poolAddress = "toto";
    walletAddress = "ccx7";

    

    if (poolAddress.IsEmpty() || walletAddress.IsEmpty())
    {
        return std::make_tuple(true, std::string(), std::string());  //TODO: change to false when pools.txt is fixed
    }

    return std::make_tuple(true, poolAddress.ToStdString(), walletAddress.ToStdString());
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
    auto [fileReachable, pool, wallet] = readMiningConfig();
    
    if (fileReachable == false)
    {
        wxMessageBox("The pools.txt file is missing or invalid in the /bin folder.\n"
                     "Please create this file by running xmr-stak for initial setup,\n"
                     "before starting the miner this way.",
                     "Configuration Missing",
                     wxOK | wxICON_ERROR);
        return false;
    }
     {
        wxMessageBox("The pools.txt file is has been detect,"
                     + pool + " " + wallet,
                     "Configuration",
                     wxOK | wxICON_INFORMATION);
    }
    MiningConfigFrame* frame = new MiningConfigFrame(nullptr, wxID_ANY, "XMR-Stak Day2Day Mining", wxDefaultPosition, wxSize(400, 300));
    frame->SetPoolInfo(pool, wallet);  // Assuming you have this method in MiningConfigFrame
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(GUIApp);
