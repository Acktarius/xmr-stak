#include "gui-ccx.hpp"
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
#include <wx/txtstrm.h>
#include <wx/process.h>
#include "ccx_art.hpp"
#include "pool_reader.hpp"

using xmrstak::config::PoolConfig;

enum {
    ID_START_BUTTON = wxID_HIGHEST + 1,
    ID_MODIFY_BUTTON,
    ID_PROCESS_OUTPUT = wxID_HIGHEST + 100,
    ID_STOP_BUTTON,
    ID_HASH_BUTTON,
    ID_RESULT_BUTTON,
    ID_CONNECT_BUTTON
};

// Implement the member functions of MiningConfigFrame
MiningConfigFrame::MiningConfigFrame(wxWindow* parent, wxWindowID id, const wxString& title, 
                                     const wxPoint& pos, const wxSize& size)
    : wxFrame(parent, id, title, pos, size)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Text displays
    m_poolText = new wxStaticText(this, wxID_ANY, "Mining Pool: ");
    m_walletText = new wxStaticText(this, wxID_ANY, "Wallet: ");
    
    mainSizer->Add(m_poolText, 0, wxALL, 10);
    mainSizer->Add(m_walletText, 0, wxALL, 10);
    // Buttons
    m_modifyButton = new wxButton(this, ID_MODIFY_BUTTON, "Modify Settings");
    m_startButton = new wxButton(this, ID_START_BUTTON, "Start Mining");
    // Stop Button
    m_stopButton = new wxButton(this, ID_STOP_BUTTON, "Stop Mining");
    m_stopButton->Hide(); // Initially hidden
    m_hashButton = new wxButton(this, ID_HASH_BUTTON, "Hashrate");
    m_hashButton->Hide();
    m_resultButton = new wxButton(this, ID_RESULT_BUTTON, "Results");
    m_resultButton->Hide();
    m_connectButton = new wxButton(this, ID_CONNECT_BUTTON, "Connection");
    m_connectButton->Hide();
    // Create a horizontal sizer for buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Add buttons to horizontal sizer
    buttonSizer->Add(m_modifyButton, 1, wxEXPAND | wxRIGHT, 5);
    buttonSizer->Add(m_startButton, 1, wxEXPAND | wxLEFT, 5);
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 10);
    // Add the console output
    m_consoleOutput = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    // Set a monospace font
    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_consoleOutput->SetFont(monoFont);
    mainSizer->Add(m_consoleOutput, 1, wxEXPAND | wxALL, 10);
    
    wxBoxSizer* buttonMiningSizer = new wxBoxSizer(wxHORIZONTAL);
    // Add stop button below console
    buttonMiningSizer->Add(m_hashButton, 1, wxEXPAND | wxALL, 10);
    buttonMiningSizer->Add(m_resultButton, 1, wxEXPAND | wxALL, 10);
    buttonMiningSizer->Add(m_connectButton, 1, wxEXPAND | wxALL, 10);
    buttonMiningSizer->Add(m_stopButton, 1, wxEXPAND | wxALL, 10);
    
    // Add the horizontal button sizer to the main vertical sizer
    mainSizer->Add(buttonMiningSizer, 0, wxALL | wxEXPAND, 10);
    
    // Bind events
    Bind(wxEVT_BUTTON, &MiningConfigFrame::OnModify, this, ID_MODIFY_BUTTON);
    Bind(wxEVT_BUTTON, 
        wxCommandEventHandler(MiningConfigFrame::OnStart), 
        this, ID_START_BUTTON);
    Bind(wxEVT_BUTTON, 
        wxCommandEventHandler(MiningConfigFrame::OnStop), 
        this, ID_STOP_BUTTON);
    Bind(wxEVT_BUTTON, &MiningConfigFrame::OnHash, this, ID_HASH_BUTTON);
    Bind(wxEVT_BUTTON, &MiningConfigFrame::OnResult, this, ID_RESULT_BUTTON);
    Bind(wxEVT_BUTTON, &MiningConfigFrame::OnConnect, this, ID_CONNECT_BUTTON);
    
    
    SetSizer(mainSizer);
    
    // Initialize process pointer
    m_process = nullptr;
    
    // Bind process output event
    Bind(wxEVT_END_PROCESS, &MiningConfigFrame::OnProcessTerminate, this, wxID_ANY);
}

void MiningConfigFrame::UpdateDisplay()
{
    m_poolText->SetLabel("Mining Pool: " + wxString(m_pool));
    m_walletText->SetLabel("Wallet: " + wxString(m_wallet));
    Layout(); // Ensure the frame updates properly
}

void MiningConfigFrame::OnModify(wxCommandEvent& event)
{
    wxTextEntryDialog poolDialog(this, "Enter new pool address:", "Modify Pool", m_pool);
    if (poolDialog.ShowModal() == wxID_OK) {
        m_pool = poolDialog.GetValue().ToStdString();
    }
    
    wxTextEntryDialog walletDialog(this, "Enter new wallet address:", "Modify Wallet", m_wallet);
    if (walletDialog.ShowModal() == wxID_OK) {
        m_wallet = walletDialog.GetValue().ToStdString();
    }
    
    UpdateDisplay();
}
// ------------------------------------------------------------------------------------------------- < OnStart Mining
void MiningConfigFrame::OnStart(wxCommandEvent& event)
{
    m_consoleOutput->Clear();
    
    if (m_process) {
        delete m_process;
    }
    
    m_process = new wxProcess(this);
    m_process->Redirect();
    
    wxString cmd = wxString::Format("./xmr-stak");
    
    long pid = wxExecute(cmd, wxEXEC_ASYNC | wxEXEC_NOHIDE, m_process);
    
    if (pid > 0) {
        // Disable modify and start buttons
        m_modifyButton->Disable();
        m_startButton->Disable();
        // Show and enable stop button
        m_stopButton->Show();
        m_hashButton->Show();
        m_resultButton->Show();
        m_connectButton->Show();
        Layout(); // Refresh layout to show stop button
        
        // Create and connect timer
        wxTimer* timer = new wxTimer(this);
        Bind(wxEVT_TIMER, &MiningConfigFrame::OnProcessTimer, this);
        timer->Start(100);
    } else {
        m_consoleOutput->AppendText("Failed to start mining process\n");
    }
}

void MiningConfigFrame::OnProcessTimer(wxTimerEvent& event)
{
    if (m_process) {
        wxInputStream* processOutput = m_process->GetInputStream();
        if (processOutput && processOutput->CanRead()) {
            wxTextInputStream tis(*processOutput);
            wxString line;
            
            // Read all available output
            while (processOutput->CanRead()) {
                line = tis.ReadLine();
                if (!line.empty()) {
                    m_consoleOutput->AppendText(line + "\n");
                }
            }
        }
        
        // Also check for error output
        wxInputStream* processError = m_process->GetErrorStream();
        if (processError && processError->CanRead()) {
            wxTextInputStream tis(*processError);
            wxString line;
            
            while (processError->CanRead()) {
                line = tis.ReadLine();
                if (!line.empty()) {
                    m_consoleOutput->AppendText("ERROR: " + line + "\n");
                }
            }
        }
    }
}
// ------------------------------------------------------------------------------------------------- < Closing frame = stop Mining
void MiningConfigFrame::OnProcessTerminate(wxProcessEvent& event)
{
    if (m_process) {
        wxKill(m_process->GetPid(), wxSIGTERM);
        wxMilliSleep(2000);  // Give more time for process to terminate
        
        // If process is still running, force kill it
        if (wxProcess::Exists(m_process->GetPid())) {
            wxKill(m_process->GetPid(), wxSIGKILL);  // SIGKILL is stronger than SIGTERM
        }
        
        delete m_process;
        m_process = nullptr;
    }
}

void MiningConfigFrame::OnStop(wxCommandEvent& event)
{
    if (m_process) {
        std::string logo = ccx_art::getStopLogo();

        m_consoleOutput->AppendText(wxString::FromUTF8(logo.c_str()));   // Convert std::string to const char*
        m_consoleOutput->SetInsertionPoint(m_consoleOutput->GetLastPosition());
        
        // Force immediate update of the console
        m_consoleOutput->Update();
        m_consoleOutput->Refresh();
        
        wxMilliSleep(1000);
        wxKill(m_process->GetPid(), wxSIGTERM);
    }
}

void MiningConfigFrame::OnHash(wxCommandEvent& event)
{
    if (m_process && m_process->GetOutputStream()) {
        wxTextOutputStream tos(*m_process->GetOutputStream());
        tos.WriteString("h\n");
        tos.Flush();
    }
}

void MiningConfigFrame::OnResult(wxCommandEvent& event)
{
    if (m_process && m_process->GetOutputStream()) {
        wxTextOutputStream tos(*m_process->GetOutputStream());
        tos.WriteString("r\n");
        tos.Flush();
    }
}

void MiningConfigFrame::OnConnect(wxCommandEvent& event)
{
    if (m_process && m_process->GetOutputStream()) {
        wxTextOutputStream tos(*m_process->GetOutputStream());
        tos.WriteString("c\n");
        tos.Flush();
    }
}

bool MiningConfigFrame::Destroy()
{
    if (m_process) {
        wxProcessEvent evt;
        OnProcessTerminate(evt);
    }
    return wxFrame::Destroy();
}

// Implement the member functions of MyApp
bool GUIApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    PoolConfig config = xmrstak::config::readMiningConfig();
    
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
        "XMR-Stak-gui-CCX", wxDefaultPosition, wxSize(1000, 800));
    frame->SetPoolInfo(config.getPoolAddress(), config.getWalletAddress());
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(GUIApp);
