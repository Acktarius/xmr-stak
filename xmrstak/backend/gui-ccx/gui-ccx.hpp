#pragma once

#include "pool_reader.hpp"
#include <wx/wx.h>
#include <tuple>
#include <string>
#include <wx/process.h>

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

    virtual bool Destroy() override;

private:
    void OnStart(wxCommandEvent& event);
    void OnModify(wxCommandEvent& event);
    void OnValidate(wxCommandEvent& event);
    void UpdateDisplay();
    void OnProcessTerminate(wxProcessEvent& event);
    void OnProcessTimer(wxTimerEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnHash(wxCommandEvent& event);
    void OnResult(wxCommandEvent& event);
    void OnConnect(wxCommandEvent& event);
    void OnProcessOutput(wxCommandEvent& event);
    std::string m_pool;
    std::string m_wallet;
    wxStaticText* m_poolText;
    wxStaticText* m_walletText;
    wxTextCtrl* m_consoleOutput;
    wxProcess* m_process;
    wxButton* m_modifyButton;
    wxButton* m_startButton;
    wxButton* m_stopButton;
    wxButton* m_hashButton;
    wxButton* m_resultButton;
    wxButton* m_connectButton;
    wxButton* m_validateButton;
    wxRadioBox* m_poolRadio;
};

class GUIApp : public wxApp
{
public:
    virtual bool OnInit() override;

private:
    // You might want to add any private members here if needed
};
