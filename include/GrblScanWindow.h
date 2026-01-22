#pragma once
#include <wx/wx.h>
#include <thread>
#include <atomic>
#include "GrblController.h"

class GrblScanWindow : public wxDialog {
public:
    GrblScanWindow(wxWindow* parent, GrblController* controller);
    ~GrblScanWindow();

private:
    GrblController* m_controller;
    
    // UI Controls
    wxTextCtrl* m_txtStartX;
    wxTextCtrl* m_txtStartY;
    wxTextCtrl* m_txtRows;
    wxTextCtrl* m_txtCols;
    wxTextCtrl* m_txtStepX;
    wxTextCtrl* m_txtStepY;
    wxTextCtrl* m_txtSpeed;
    wxRadioBox* m_rbDirection;
    wxButton* m_btnStart;
    wxButton* m_btnClose;

    // Threading
    std::thread m_workerThread;
    std::atomic<bool> m_isScanning{false};

    // Events
    void OnStart(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnBtnClose(wxCommandEvent& event);

    // Helpers
    void ToggleControls(bool enable);

    wxDECLARE_EVENT_TABLE();
};