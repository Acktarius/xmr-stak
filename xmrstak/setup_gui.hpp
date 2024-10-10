#pragma once

#include <wx/wx.h>

class MiningConfigFrame : public wxFrame
{
public:
    MiningConfigFrame(wxWindow* parent, wxWindowID id, const wxString& title, 
                    const wxPoint& pos = wxDefaultPosition, 
                    const wxSize& size = wxDefaultSize);

private:

    void OnStart(wxCommandEvent& event);

    // Other private members and methods
};

class GUIApp : public wxApp
{
public:
    virtual bool OnInit() override;
};
