#pragma once

#include "GrblController.hpp"
#include "ScanHandler.hpp"
#include "AppSettings.hpp"
#include "wxHelpers.hpp"
#include <thread>
#include <atomic>
#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/valnum.h>
#include <wx/valgen.h>
#include <functional>
#include <memory>

class GrblScanWindow : public wxPanel {
public:
    using PreviewCallback = std::function<void(const std::vector<ScanLine>&)>;

    GrblScanWindow(wxWindow* parent, std::shared_ptr<ScanHandler> controller, PreviewCallback onPreviewUpdate, GridPatternSettings* initialSettings);
    ~GrblScanWindow();

    void SetSettings(GridPatternSettings& pattern);

private:
    std::shared_ptr<ScanHandler> m_controller;
    
    // UI Controls
    wxTextCtrl* m_txtStartX;
    wxTextCtrl* m_txtStartY;
    wxTextCtrl* m_txtRows;
    wxTextCtrl* m_txtCols;
    wxTextCtrl* m_txtStepX;
    wxTextCtrl* m_txtStepY;
    wxTextCtrl* m_txtSpeed;
    wxRadioBox* m_rbDirection;
    wxCheckBox* m_chkZigzag;
    wxCheckBox* m_chkContinuous;
    wxTextCtrl* m_txtLeadDistance;
    wxButton* m_btnStart;

    // Called on every keystroke — just resets the debounce timer
    void OnUIChange();

    // Called when the debounce timer fires — does the actual heavy work
    void OnPreviewTimerFired(wxTimerEvent& event);

    // Rebinds all validators to point at the current m_settings
    void RebindValidators();

    PreviewCallback OnPreviewUpdate;

    GridPatternSettings* m_settings;

    // Debounce timer for preview updates
    wxTimer m_previewTimer;
    static const int PREVIEW_TIMER_ID = 2001;
    static const int PREVIEW_DELAY_MS = 150;

    // Threading
    std::thread m_workerThread;
    std::atomic<bool> m_isScanning{false};

    // Events
    void OnStart(wxCommandEvent& event);

    // Helpers
    void ToggleControls(bool enable);

    wxDECLARE_EVENT_TABLE();
};