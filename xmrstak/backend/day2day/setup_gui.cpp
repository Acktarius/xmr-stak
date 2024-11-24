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
#include <wx/txtstrm.h>
#include <wx/process.h>


enum {
    ID_START_BUTTON = wxID_HIGHEST + 1,
    ID_MODIFY_BUTTON,
    ID_BIND_BUTTON,
    ID_PROCESS_OUTPUT = wxID_HIGHEST + 100,
    ID_STOP_BUTTON
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
    
    // Text displays
    m_poolText = new wxStaticText(this, wxID_ANY, "Mining Pool: ");
    m_walletText = new wxStaticText(this, wxID_ANY, "Wallet: ");
    
    mainSizer->Add(m_poolText, 0, wxALL, 10);
    mainSizer->Add(m_walletText, 0, wxALL, 10);
    
    // Buttons
    m_modifyButton = new wxButton(this, ID_MODIFY_BUTTON, "Modify Settings");
    m_startButton = new wxButton(this, ID_START_BUTTON, "Start Mining");
    m_stopButton = new wxButton(this, ID_STOP_BUTTON, "Stop Mining");
    m_stopButton->Hide(); // Initially hidden
    
    // Create a horizontal sizer for buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Add buttons to horizontal sizer
    buttonSizer->Add(m_modifyButton, 1, wxEXPAND | wxRIGHT, 5);
    buttonSizer->Add(m_startButton, 1, wxEXPAND | wxLEFT, 5);
    
    // Add the console output
    m_consoleOutput = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    mainSizer->Add(m_consoleOutput, 1, wxEXPAND | wxALL, 10);
    
    // Add stop button below console
    mainSizer->Add(m_stopButton, 0, wxEXPAND | wxALL, 10);
    
    // Add the horizontal button sizer to the main vertical sizer
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 10);
    
    // Bind events
    Bind(wxEVT_BUTTON, &MiningConfigFrame::OnModify, this, ID_MODIFY_BUTTON);
    Bind(wxEVT_BUTTON, 
        wxCommandEventHandler(MiningConfigFrame::OnStart), 
        this, ID_START_BUTTON);
    Bind(wxEVT_BUTTON, 
        wxCommandEventHandler(MiningConfigFrame::OnStop), 
        this, ID_STOP_BUTTON);
    
    
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

void MiningConfigFrame::OnBind(wxCommandEvent& event)
{
    wxExecute("./xmr-stak --bind", wxEXEC_SYNC);
    wxMessageBox("GPU Binding Complete", "Bind GPU", wxOK | wxICON_INFORMATION);
}

void MiningConfigFrame::OnProcessTerminate(wxProcessEvent& event)
{
    if (m_process) {
        wxInputStream* processOutput = m_process->GetInputStream();
        wxTextInputStream tis(*processOutput);
        
        while (processOutput->CanRead()) {
            wxString line = tis.ReadLine();
            m_consoleOutput->AppendText(line + "\n");
        }
        
        delete m_process;
        m_process = nullptr;
        
        // Re-enable buttons and hide stop button
        m_modifyButton->Enable();
        m_startButton->Enable();
        m_stopButton->Hide();
        Layout(); // Refresh layout
    }
}

void MiningConfigFrame::OnStop(wxCommandEvent& event)
{
    if (m_process) {
        // Kill the process
        wxKill(m_process->GetPid(), wxSIGTERM);
        m_consoleOutput->AppendText("Stopping mining process...\n");
    }
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
        "XMR-Stak-CCX", wxDefaultPosition, wxSize(1000, 800));
    frame->SetPoolInfo(config.getPoolAddress(), config.getWalletAddress());
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(GUIApp);
