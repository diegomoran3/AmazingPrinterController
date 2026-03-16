#pragma once

#include "GrblController.hpp"
#include "AppSettings.hpp"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>
#include <wx/statline.h>
#include <wx/artprov.h>

#include <memory>
#include <functional>

class GrblControlPanel : public wxPanel {
public:
    GrblControlPanel(wxWindow* parent, std::shared_ptr<GrblController> grbl, JogSettings* settings);

    double GetStepSize() const;
    double GetFeedRate() const;

private:
    std::shared_ptr<GrblController> m_grbl;
    JogSettings* m_settings;

    // UI Controls
    wxSpinCtrlDouble* m_stepSizeCtrl;
    wxSpinCtrl*       m_feedRateCtrl;

    wxTextCtrl* m_xInput;
    wxTextCtrl* m_yInput;
    wxButton*   m_gotoBtn;

    // Builder helpers
    wxSizer* CreateControlBox(wxWindow* parent);
    wxSizer* CreateMotionBox(wxWindow* parent);

    // Event Handlers
    void OnGoTo(wxCommandEvent& event);
    void OnHome(wxCommandEvent& event);
    void OnUnlock(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);
    void OnResume(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnJog(wxCommandEvent& event);

    // Event IDs
    enum {
        ID_GOTO = 4,
        ID_HOME = 5,
        ID_UNLOCK = 6,
        ID_PAUSE = 7,
        ID_RESUME = 8,
        ID_RESET = 9,
        ID_JOG_UP = 100,
        ID_JOG_DOWN = 101,
        ID_JOG_LEFT = 102,
        ID_JOG_RIGHT = 103,
        ID_JOG_UP_LEFT = 104,
        ID_JOG_UP_RIGHT = 105,
        ID_JOG_DOWN_LEFT = 106,
        ID_JOG_DOWN_RIGHT = 107
    };

    wxDECLARE_EVENT_TABLE();
};