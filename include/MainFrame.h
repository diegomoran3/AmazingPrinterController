#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <memory>
#include "CoordinatePanel.h"
#include "GrblController.h"

class MainFrame : public wxFrame
{
public:
    MainFrame();

private:
    // Event Handlers
    void OnConnect(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
    void OnGoTo(wxCommandEvent& event);

    // Helper Methods
    void UpdatePortList();

    // UI Components
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
        ID_GOTO = 4
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_H
