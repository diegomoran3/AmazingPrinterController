#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "CoordinatePanel.hpp"
#include "GrblController.hpp"
#include "GrblConfigDialog.hpp"
#include "GrblScanWindow.hpp"
#include "AppSettings.hpp"

#include <wx/wx.h>
#include <wx/spinctrl.h> 
#include <wx/statline.h>
#include <wx/splitter.h>
#include <wx/artprov.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>

#include <memory>
#include <functional>

struct PreviewRegion {
    double x;
    double y;
    double width;
    double height;
};

class MainFrame : public wxFrame
{
public:
    MainFrame();

    void SetPreviewRegion(double x, double y, double width, double height);

private:
    GrblConfigDialog* m_configDlg = nullptr; 
    GrblScanWindow* m_scanDlg = nullptr;

    void OnOpenSettings(wxCommandEvent& event);

    // Helper methods (Updated signature to take specific parents)
    void BuildLeftPanel(wxPanel* parent);
    void BuildManualControlTab(wxPanel *parent);
    void BuildRightPanel(wxPanel *parent);

    // Sub-components of the Left Column
    wxSizer* CreateConnectionBox(wxPanel* parent);
    wxSizer* CreateMotionBox(wxPanel* parent);
    void     SetupLogArea(wxPanel* parent, wxBoxSizer* mainVerticalSizer); // Adds directly to sizer
    
    // Logic setup
    void SetupGrblCallbacks();

    wxSizer* CreateControlBox(wxPanel* parent);

    // Event Handlers
    void OnConnect(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
    void OnGoTo(wxCommandEvent& event);
    void OnHome(wxCommandEvent& event);
    void OnUnlock(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);
    void OnResume(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);

    void OnJog(wxCommandEvent &event);

    // Helper Methods
    void UpdatePortList();

    // UI Components
    wxSplitterWindow* m_splitter;
    wxPanel* m_leftPanel;
    wxPanel* m_rightPanel;

    wxComboBox* m_portCombo;
    wxButton* m_connectBtn;
    wxButton* m_refreshBtn;
    wxTextCtrl* m_logCtrl;

    wxTextCtrl* m_cmdInput;
    wxButton* m_sendBtn;

    CoordinatePanel* m_coordPanel = nullptr;
    wxTextCtrl* m_xInput;
    wxTextCtrl* m_yInput;
    wxButton* m_gotoBtn;

    wxSpinCtrlDouble* m_stepSizeCtrl;
    wxSpinCtrl* m_feedRateCtrl;

    wxNotebook* m_sidebarTabs;

    GrblScanWindow* m_scanPanel;

    PreviewRegion m_currentPreviewRegion;

    AppSettings m_settings;

    std::shared_ptr<GrblController> m_grbl;

    // Event IDs
    enum {
        ID_CONNECT = 1,
        ID_REFRESH = 2,
        ID_SEND = 3,
        ID_GOTO = 4,
        ID_HOME = 5,
        ID_UNLOCK = 6,
        ID_PAUSE = 7,
        ID_RESUME = 8,
        ID_RESET = 9,
        ID_SETTINGS_TOOL = 50,
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

#endif // MAINFRAME_H
