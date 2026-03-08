#pragma once

#include "GrblController.hpp"
#include "ScanHandler.hpp"
#include "AppSettings.hpp"
#include <thread>
#include <atomic>
#include <wx/wx.h>
#include <wx/valnum.h>
#include <wx/valgen.h>
#include <functional>
#include <memory>

class GrblScanWindow : public wxPanel {
public:
    using PreviewCallback = std::function<void(double, double, double, double)>;

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
    wxButton* m_btnStart;

    void AddInputDouble(wxFlexGridSizer* sizer, const wxString& label, wxTextCtrl*& ptr, double* dataPtr, double minVal = 0.0);
    void AddInputInt(wxFlexGridSizer* sizer, const wxString& label, wxTextCtrl*& ptr, int* dataPtr, int minVal = 1);

    void OnUIChange();

    PreviewCallback OnPreviewUpdate;

    GridPatternSettings* m_settings;

    // Threading
    std::thread m_workerThread;
    std::atomic<bool> m_isScanning{false};

    // Events
    void OnStart(wxCommandEvent& event);

    // Helpers
    void ToggleControls(bool enable);

    wxDECLARE_EVENT_TABLE();
};