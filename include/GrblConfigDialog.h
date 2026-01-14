#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include "GrblController.h"

class GrblConfigDialog : public wxDialog {
public:
    GrblConfigDialog(wxWindow* parent, GrblController* controller);
    void ReloadGrid();
    void ParseLine(const std::string& line);

private:
    GrblController* m_controller;
    wxGrid* m_grid;

    void OnRefresh(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
