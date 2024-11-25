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
#include <wx/radiobox.h>
#include <wx/string.h>
#include <wx/txtstrm.h>
#include <wx/process.h>
#include "ccx_art.hpp"
#include "pool_reader.hpp"
#include "KccxPools.hpp"

using xmrstak::config::PoolConfig;

enum {
    ID_START_BUTTON = wxID_HIGHEST + 1,
    ID_MODIFY_BUTTON,
    ID_VALIDATE_BUTTON,
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
    // Buttons_Pool config
    m_modifyButton = new wxButton(this, ID_MODIFY_BUTTON, "Modify Settings");
    m_validateButton = new wxButton(this, ID_VALIDATE_BUTTON, "Validate");
    m_validateButton->Hide();
    // Buttons_Mining
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
    m_consoleOutput->Hide();
    mainSizer->Add(m_consoleOutput, 1, wxEXPAND | wxALL, 10);
    // Add the Radio box

    // Create radio box with initial empty choices
    wxArrayString choices;
    m_poolRadio = new wxRadioBox(this, wxID_ANY, "Available Pools",
                                wxDefaultPosition, wxDefaultSize,
                                choices, 1, wxRA_VERTICAL);
    m_poolRadio->Hide(); // Initially hidden

    // Add to main sizer
    mainSizer->Add(m_poolRadio, 0, wxEXPAND | wxALL, 10);

    // Add validate button (already declared in header)
    mainSizer->Add(m_validateButton, 0, wxEXPAND | wxALL, 10);
    
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
    Bind(wxEVT_BUTTON, &MiningConfigFrame::OnValidate, this, ID_VALIDATE_BUTTON);
    
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
    m_startButton->Hide();
    m_validateButton->Show();
    
    try {
        // Load known pools from JSON
        std::vector<KPool> pools = loadKnownPools("./KccxPools.json");
        if (pools.empty()) {
            wxMessageBox("No pools were loaded from the JSON file", "Warning", wxICON_WARNING);
        }
        
        // Update radio box choices
        wxArrayString choices;
        for (const auto& pool : pools) {
            choices.Add(wxString::Format("%s (%s:%d)", 
                pool.name.c_str(), pool.url.c_str(), pool.port));
        }

        // Properly recreate the radio box
        if (m_poolRadio) {
            m_poolRadio->Destroy();
            m_poolRadio = new wxRadioBox(this, wxID_ANY, "Available Pools",
                                        wxDefaultPosition, wxDefaultSize,
                                        choices, 1, wxRA_VERTICAL);
        }

        m_poolRadio->Show();
        
        Layout(); // Refresh the layout
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error loading pools: %s", e.what()), "Error", wxICON_ERROR);
    }
     UpdateDisplay();
}

// ------------------------------------------------------------------------------------------------- < OnStart Mining
void MiningConfigFrame::OnStart(wxCommandEvent& event)
{
    m_consoleOutput->Show();
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
// ------------------------------------------------------------------------------------------------- < Stop Mining
void MiningConfigFrame::OnStop(wxCommandEvent& event)
{
    if (m_process) {
        // Display the stop logo
        std::string logo = ccx_art::getStopLogo();
        m_consoleOutput->AppendText(wxString::FromUTF8(logo.c_str()));
        m_consoleOutput->SetInsertionPoint(m_consoleOutput->GetLastPosition());
        
        // Force immediate update of the console
        m_consoleOutput->Update();
        m_consoleOutput->Refresh();
        wxYield(); // Allow the GUI to process pending events
        
        // Wait to show the logo
        wxMilliSleep(1000);
        
        // Terminate the mining process
        wxKill(m_process->GetPid(), wxSIGTERM);
        
        // Wait for process cleanup
        wxYield(); // Process any pending events
        wxMilliSleep(5000);
        
        // Hide UI elements
        m_consoleOutput->Hide();
        m_hashButton->Hide();
        m_resultButton->Hide();
        m_connectButton->Hide();
        m_stopButton->Hide();
        m_modifyButton->Enable();
        m_startButton->Enable();
        UpdateDisplay();
    }
}
// ------------------------------------------------------------------------------------------------- < other Events
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

void MiningConfigFrame::OnValidate(wxCommandEvent& event)
{
    // Get selected pool from radio box
    int selection = m_poolRadio->GetSelection();
    if (selection != wxNOT_FOUND) {
        wxString selectedPool = m_poolRadio->GetString(selection);
        
        // TODO: Add your validation logic here
        // For example:
        // 1. Parse the selected pool string
        // 2. Validate the connection
        // 3. Update the configuration
        
        // Show success message
        wxMessageBox("Pool selection validated!", "Success", wxICON_INFORMATION);
        
        // Hide validation UI elements and show mining controls
        m_poolRadio->Hide();
        m_validateButton->Hide();
        m_startButton->Show();
        
        Layout(); // Refresh the layout
    } else {
        wxMessageBox("Please select a pool first", "Error", wxICON_ERROR);
    }
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
