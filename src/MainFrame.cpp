#include "MainFrame.h"

// Event Table
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_CONNECT, MainFrame::OnConnect)
    EVT_BUTTON(ID_REFRESH, MainFrame::OnRefresh)
    EVT_BUTTON(ID_SEND, MainFrame::OnSend)
    EVT_TEXT_ENTER(ID_SEND, MainFrame::OnSend)
    EVT_BUTTON(ID_GOTO, MainFrame::OnGoTo)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "Amazing Printer Controller", wxDefaultPosition, wxSize(600, 500)),
      m_grbl(std::make_unique<GrblController>())
{
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL); 

    // LEFT SIDE: Controls and Console
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // --- PORT SELECTION AREA ---
    wxStaticBoxSizer* portSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Connection Settings");

    m_portCombo = new wxComboBox(portSizer->GetStaticBox(), wxID_ANY, "",
                                 wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);

    m_refreshBtn = new wxButton(portSizer->GetStaticBox(), ID_REFRESH, "Refresh");
    m_connectBtn = new wxButton(portSizer->GetStaticBox(), ID_CONNECT, "Connect");

    portSizer->Add(m_portCombo, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_refreshBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_connectBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    // --- COORDINATE INPUT AREA --- 
    wxStaticBoxSizer* coordInputSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Go To Position");
    
    m_xInput = new wxTextCtrl(coordInputSizer->GetStaticBox(), wxID_ANY, "0");
    m_yInput = new wxTextCtrl(coordInputSizer->GetStaticBox(), wxID_ANY, "0");
    m_gotoBtn = new wxButton(coordInputSizer->GetStaticBox(), ID_GOTO, "Go To");
    
    coordInputSizer->Add(new wxStaticText(coordInputSizer->GetStaticBox(), wxID_ANY, "X:"), 
                         0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    coordInputSizer->Add(m_xInput, 1, wxALL, 5);
    coordInputSizer->Add(new wxStaticText(coordInputSizer->GetStaticBox(), wxID_ANY, "Y:"), 
                         0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    coordInputSizer->Add(m_yInput, 1, wxALL, 5);
    coordInputSizer->Add(m_gotoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    // --- LOG AREA ---
    m_logCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    m_logCtrl->SetBackgroundColour(wxColour(30, 30, 30));
    m_logCtrl->SetForegroundColour(wxColour(0, 255, 0));

    // Set up standard logging to redirect to this box
    wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));

    wxBoxSizer* cmdSizer = new wxBoxSizer(wxHORIZONTAL);

    // Create text input with TE_PROCESS_ENTER so pressing 'Enter' triggers the event
    m_cmdInput = new wxTextCtrl(panel, ID_SEND, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_sendBtn = new wxButton(panel, ID_SEND, "Send");

    cmdSizer->Add(m_cmdInput, 1, wxEXPAND | wxRIGHT, 5);
    cmdSizer->Add(m_sendBtn, 0, wxEXPAND);

    // Combine left side
    leftSizer->Add(portSizer, 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(coordInputSizer, 0, wxEXPAND | wxALL, 5);  // NEW
    leftSizer->Add(new wxStaticText(panel, wxID_ANY, "Console Output:"), 0, wxLEFT | wxTOP, 5);
    leftSizer->Add(m_logCtrl, 1, wxEXPAND | wxALL, 5);
    leftSizer->Add(cmdSizer, 0, wxEXPAND | wxALL, 5);

    // RIGHT SIDE: Coordinate Panel
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    rightSizer->Add(new wxStaticText(panel, wxID_ANY, "Coordinate System:"), 0, wxLEFT | wxTOP, 10);
    
    m_coordPanel = new CoordinatePanel(panel);
    m_coordPanel->SetMinSize(wxSize(400, 400));  // Minimum size for visibility
    rightSizer->Add(m_coordPanel, 1, wxEXPAND | wxALL, 10);

    // Combine left and right
    mainSizer->Add(leftSizer, 1, wxEXPAND);
    mainSizer->Add(rightSizer, 1, wxEXPAND);

    panel->SetSizer(mainSizer);

    // Initial Port Scan
    m_grbl->SetOnMessageReceived([this](const std::string& line) {
        this->CallAfter([this, line]() {
            wxLogMessage("RX: %s", line);
        });
    });

    m_grbl->SetOnStatusUpdate([this](const GrblStatus& status) {
    this->CallAfter([this, status]() {
            m_coordPanel->ClearPoints();
            m_coordPanel->AddPoint(status.x, status.y);
        });
    });

    UpdatePortList();

    Centre();
}

void MainFrame::UpdatePortList() {
    m_portCombo->Clear();
    auto ports = m_grbl->GetAvailablePorts();
    for (const auto& port : ports) m_portCombo->Append(port);
    
    if (!ports.empty()) m_portCombo->SetSelection(0);
}

void MainFrame::OnRefresh(wxCommandEvent& event) {
    UpdatePortList();
}

void MainFrame::OnConnect(wxCommandEvent& event) {
    if (m_grbl->IsConnected()) {
        m_grbl->Disconnect();
        m_connectBtn->SetLabel("Connect");
        m_portCombo->Enable(true);
        wxLogMessage("Disconnected.");
    } else {
        wxString selectedPort = m_portCombo->GetStringSelection();
        if (m_grbl->Connect(selectedPort.ToStdString())) {
            m_connectBtn->SetLabel("Disconnect");
            m_portCombo->Enable(false);
            wxLogMessage("Successfully connected to %s", selectedPort);
        } else {
            wxLogError("Failed to open port %s", selectedPort);
        }
    }
}

void MainFrame::OnSend(wxCommandEvent& event) {
    if (!m_grbl->IsConnected()) return;

    wxString cmd = m_cmdInput->GetValue();
    if (cmd.IsEmpty()) return;

    wxLogMessage("TX: %s", cmd);
    m_grbl->SendRawCommand(cmd.ToStdString());
    m_cmdInput->Clear();
}

void MainFrame::OnGoTo(wxCommandEvent& event) {
    double x, y;
    if (!m_xInput->GetValue().ToDouble(&x) || !m_yInput->GetValue().ToDouble(&y)) {
        wxLogError("Invalid coordinates!");
        return;
    }
    
    // Abstracted logic: we just say MoveTo, we don't care about "G0" or "\n"
    m_grbl->MoveTo(x, y);
    wxLogMessage("Moving to X:%.2f Y:%.2f", x, y);
}