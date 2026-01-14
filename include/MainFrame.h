#ifndef MAINFRAME_H
#define MAINFRAME_H

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>

#include <wx/wx.h>
#include <wx/splitter.h>
#include <memory>
#include "CoordinatePanel.h"
#include "GrblController.h"

class MainFrame : public wxFrame
{
public:
    MainFrame();

private:
    // Helper methods (Updated signature to take specific parents)
    void BuildLeftPanel(wxPanel* parent);
    void BuildRightPanel(wxPanel* parent);
    
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

    CoordinatePanel* m_coordPanel;
    wxTextCtrl* m_xInput;
    wxTextCtrl* m_yInput;
    wxButton* m_gotoBtn;

    // Logic - Replaced SerialPortManager with GrblController
    std::unique_ptr<GrblController> m_grbl;

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
        ID_RESET = 9
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_H
