#pragma once

#include "AppSettings.hpp"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <functional>

class AppSettingsPanel : public wxPanel {
public:
    using SettingsChangedCallback = std::function<void()>;

    AppSettingsPanel(wxWindow* parent, AppSettings* settings, SettingsChangedCallback onChanged = nullptr);

private:
    AppSettings* m_settings;
    SettingsChangedCallback m_onChanged;

    // Display
    wxCheckBox* m_chkShowPreview;

    // Connection
    wxCheckBox* m_chkAutoHome;

    // Polling
    wxSpinCtrl* m_spinPollRate;

    // Motion Defaults
    wxSpinCtrlDouble* m_spinDefaultStep;
    wxSpinCtrl* m_spinDefaultFeed;

    // Tolerance
    wxSpinCtrlDouble* m_spinPosTolerance;
    wxSpinCtrlDouble* m_spinArrivalTimeout;

    void OnSettingChanged();
    void BuildUI();
};