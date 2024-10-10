#include "setup_gui.hpp"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>

enum {
    ID_START_BUTTON = wxID_HIGHEST + 1
};
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

        // Check for pool.txt file
        wxString poolFilePath = wxFileName::GetCwd() + wxFileName::GetPathSeparator() + "pools.txt";

        if (!wxFileExists(poolFilePath))
        {
            wxMessageBox("The pools.txt file is missing in the /bin folder. "
                         "Please create this file running xmr-stak_initial setup before starting the miner this way.",
                         "Configuration Missing",
                         wxOK | wxICON_ERROR);
            return false;
        }

        MiningConfigFrame* frame = new MiningConfigFrame(nullptr, wxID_ANY, "XMR-Stak Day2Day Mining", wxDefaultPosition, wxSize(400, 300));
        frame->Show(true);
        return true;
    }


wxIMPLEMENT_APP(GUIApp);
