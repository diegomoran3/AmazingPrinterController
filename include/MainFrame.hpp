#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "CoordinatePanel.hpp"
#include "GrblController.hpp"
#include "GrblConfigDialog.hpp"
#include "GrblScanPanel.hpp"
#include "GrblControlPanel.hpp"
#include "AppSettingsPanel.hpp"
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
    MainFrame(AppSettings* settings);

    void SetPreviewRegion(double x, double y, double width, double height);
    void SetScanGridPreview(const std::vector<ScanLine>& scanLines);

private:
    GrblConfigDialog* m_configDlg = nullptr; 
    GrblScanPanel* m_scanDlg = nullptr;

    void OnOpenSettings(wxCommandEvent& event);

    // Helper methods
    void BuildLeftPanel(wxPanel* parent);
    void BuildRightPanel(wxPanel *parent);

    // Sub-components of the Left Column
    wxSizer* CreateConnectionBox(wxPanel* parent);
    void     SetupLogArea(wxPanel* parent, wxBoxSizer* mainVerticalSizer);
    
    // Logic setup
    void SetupGrblCallbacks();

    // Event Handlers
    void OnConnect(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);

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

    wxNotebook* m_sidebarTabs;

    GrblControlPanel* m_controlPanel;
    GrblScanPanel* m_scanPanel;
    AppSettingsPanel* m_settingsPanel;

    void ApplySettings();

    PreviewRegion m_currentPreviewRegion;

    AppSettings* m_settings;

    std::shared_ptr<GrblController> m_grbl;

    // Event IDs
    enum {
        ID_CONNECT = 1,
        ID_REFRESH = 2,
        ID_SEND = 3,
        ID_SETTINGS_TOOL = 50
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_H