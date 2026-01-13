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
// 1. Create Main Container
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    // 2. Build Columns
    wxSizer* leftSizer = CreateLeftColumn(panel);
    wxSizer* rightSizer = CreateRightColumn(panel);

    // 3. Assemble Layout
    mainSizer->Add(leftSizer, 0, wxEXPAND | wxALL, 5); // Fixed width controls
    mainSizer->Add(rightSizer, 1, wxEXPAND | wxALL, 5); // Expanding visual area
    panel->SetSizer(mainSizer);

    // 4. Setup Logic
    SetupGrblCallbacks();
    UpdatePortList();

    Centre();
}

wxSizer* MainFrame::CreateLeftColumn(wxPanel* parent)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Add modular components
    sizer->Add(CreateConnectionBox(parent), 0, wxEXPAND | wxALL, 5);
    sizer->Add(CreateMotionBox(parent), 0, wxEXPAND | wxALL, 5);
    
    // Setup Log Area adds multiple items (label, textctrl, input), 
    // so we pass the sizer to it to fill.
    SetupLogArea(parent, sizer);

    return sizer;
}

wxSizer* MainFrame::CreateRightColumn(wxPanel* parent)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    sizer->Add(new wxStaticText(parent, wxID_ANY, "Coordinate System:"), 0, wxLEFT | wxTOP, 10);

    m_coordPanel = new CoordinatePanel(parent);
    m_coordPanel->SetMinSize(wxSize(400, 400));
    
    sizer->Add(m_coordPanel, 1, wxEXPAND | wxALL, 10);

    return sizer;
}

wxSizer* MainFrame::CreateConnectionBox(wxPanel* parent)
{
    wxStaticBoxSizer* portSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Connection Settings");

    m_portCombo = new wxComboBox(portSizer->GetStaticBox(), wxID_ANY, "",
                                 wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
    m_refreshBtn = new wxButton(portSizer->GetStaticBox(), ID_REFRESH, "Refresh");
    m_connectBtn = new wxButton(portSizer->GetStaticBox(), ID_CONNECT, "Connect");

    portSizer->Add(m_portCombo, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_refreshBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_connectBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    return portSizer;
}

wxSizer* MainFrame::CreateMotionBox(wxPanel* parent)
{
    wxStaticBoxSizer* coordInputSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Go To Position");

    m_xInput = new wxTextCtrl(coordInputSizer->GetStaticBox(), wxID_ANY, "0");
    m_yInput = new wxTextCtrl(coordInputSizer->GetStaticBox(), wxID_ANY, "0");
    m_gotoBtn = new wxButton(coordInputSizer->GetStaticBox(), ID_GOTO, "Go To");

    coordInputSizer->Add(new wxStaticText(coordInputSizer->GetStaticBox(), wxID_ANY, "X:"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    coordInputSizer->Add(m_xInput, 1, wxALL, 5);
    coordInputSizer->Add(new wxStaticText(coordInputSizer->GetStaticBox(), wxID_ANY, "Y:"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    coordInputSizer->Add(m_yInput, 1, wxALL, 5);
    coordInputSizer->Add(m_gotoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    return coordInputSizer;
}

void MainFrame::SetupLogArea(wxPanel* parent, wxBoxSizer* mainVerticalSizer)
{
    // 1. Label
    mainVerticalSizer->Add(new wxStaticText(parent, wxID_ANY, "Console Output:"), 0, wxLEFT | wxTOP, 5);

    // 2. Log Window
    m_logCtrl = new wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    m_logCtrl->SetBackgroundColour(wxColour(30, 30, 30));
    m_logCtrl->SetForegroundColour(wxColour(0, 255, 0));
    
    // Redirect wxLog
    delete wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));
    
    mainVerticalSizer->Add(m_logCtrl, 1, wxEXPAND | wxALL, 5);

    // 3. Command Input Area
    wxBoxSizer* cmdSizer = new wxBoxSizer(wxHORIZONTAL);
    m_cmdInput = new wxTextCtrl(parent, ID_SEND, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_sendBtn = new wxButton(parent, ID_SEND, "Send");

    cmdSizer->Add(m_cmdInput, 1, wxEXPAND | wxRIGHT, 5);
    cmdSizer->Add(m_sendBtn, 0, wxEXPAND);

    mainVerticalSizer->Add(cmdSizer, 0, wxEXPAND | wxALL, 5);
}

void MainFrame::SetupGrblCallbacks()
{
    // Keep backend logic separate from UI construction
    m_grbl->SetOnMessageReceived([this](const std::string& line) {
        this->CallAfter([this, line]() {
            wxLogMessage("RX: %s", line);
        });
    });

    m_grbl->SetOnStatusUpdate([this](const GrblStatus& status) {
        this->CallAfter([this, status]() {
            m_coordPanel->ClearPoints();
            m_coordPanel->AddPoint(status.x, status.y, *wxBLUE);
        });
    });
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
