#include "MainFrame.h"

// Event Table
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_CONNECT,          MainFrame::OnConnect)
    EVT_BUTTON(ID_REFRESH,          MainFrame::OnRefresh)
    EVT_BUTTON(ID_SEND,             MainFrame::OnSend)
    EVT_TEXT_ENTER(ID_SEND,         MainFrame::OnSend)
    EVT_BUTTON(ID_GOTO,             MainFrame::OnGoTo)
    EVT_BUTTON(ID_HOME,             MainFrame::OnHome)
    EVT_BUTTON(ID_UNLOCK,           MainFrame::OnUnlock)
    EVT_BUTTON(ID_PAUSE,            MainFrame::OnPause)
    EVT_BUTTON(ID_RESUME,           MainFrame::OnResume)
    EVT_BUTTON(ID_RESET,            MainFrame::OnReset)
    EVT_TOOL(ID_SETTINGS_TOOL,      MainFrame::OnOpenSettings)
    EVT_BUTTON(ID_JOG_UP,           MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_DOWN,         MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_LEFT,         MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_RIGHT,        MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_UP_LEFT,      MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_UP_RIGHT,     MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_DOWN_LEFT,    MainFrame::OnJog)
    EVT_BUTTON(ID_JOG_DOWN_RIGHT,   MainFrame::OnJog)
wxEND_EVENT_TABLE()

using namespace std::placeholders;

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "Amazing Printer Controller", wxDefaultPosition, wxSize(600, 500)),
      m_grbl(std::make_unique<GrblController>())
{
    // --- Create Toolbar ---
    wxToolBar* toolbar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL);
    
    // Add a Settings Tool (You can use a custom icon, here using a standard one)
    toolbar->AddTool(ID_SETTINGS_TOOL, "GRBL Settings", 
                     wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_TOOLBAR));
                     
    toolbar->Realize();

    // 1. Create the Splitter Window as the main child of the Frame
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);

    // 2. Create the Left and Right panels as children of the Splitter
    m_leftPanel = new wxPanel(m_splitter);
    m_rightPanel = new wxPanel(m_splitter);

    // 3. Build the internals of each panel
    BuildLeftPanel(m_leftPanel);
    BuildRightPanel(m_rightPanel);

    // 4. Configure the Splitter
    m_splitter->SetMinimumPaneSize(200); // Prevent dragging too far
    m_splitter->SplitVertically(m_leftPanel, m_rightPanel, 350); // 350px is the initial position of the divider

    // 5. Logic Setup (Same as before)
    SetupGrblCallbacks();
    UpdatePortList();
    Centre();
}

void MainFrame::BuildLeftPanel(wxPanel* parent)
{
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Connection Box stays at the top (always visible)
    leftSizer->Add(CreateConnectionBox(parent), 0, wxEXPAND | wxALL, 5);

    // 2. Create Notebook for switching between Manual and Scan
    m_sidebarTabs = new wxNotebook(parent, wxID_ANY);
    
    // Create Pages
    wxPanel* manualPage = new wxPanel(m_sidebarTabs);

    // Build Manual Page (Existing logic)
    BuildManualControlTab(manualPage);

    m_scanPanel = new GrblScanWindow(
        m_sidebarTabs, 
        m_grbl.get(), 
        std::bind(&MainFrame::SetPreviewRegion, this, _1, _2, _3, _4)
    );

    m_sidebarTabs->AddPage(manualPage, "Manual");
    m_sidebarTabs->AddPage(m_scanPanel, "Scanner");

    leftSizer->Add(m_sidebarTabs, 0, wxEXPAND | wxALL, 5);

    // 3. Console stays at the bottom
    SetupLogArea(parent, leftSizer);

    parent->SetSizer(leftSizer);
}

void MainFrame::BuildManualControlTab(wxPanel* parent)
{
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // Use the specific 'parent' (m_leftPanel) for controls
    leftSizer->Add(CreateControlBox(parent), 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(CreateMotionBox(parent), 0, wxEXPAND | wxALL, 5);

    parent->SetSizer(leftSizer);
}

void MainFrame::BuildRightPanel(wxPanel* parent)
{
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    rightSizer->Add(new wxStaticText(parent, wxID_ANY, "Coordinate System:"), 0, wxLEFT | wxTOP, 10);

    // CoordinatePanel now lives inside the right panel
    m_coordPanel = new CoordinatePanel(parent);
    m_coordPanel->SetMinSize(wxSize(400, 400));

    m_coordPanel->SetOnPointClicked([this](double x, double y) {
        
        // Check connection first
        if (!m_grbl || !m_grbl->IsConnected()) {
            wxMessageBox("Connect to machine first!", "Error", wxICON_WARNING);
            return;
        }
        
        m_grbl->MoveTo(x, y);
        
        // Optional: Update the "Target" dot immediately for visual feedback
        // (The machine status update will eventually overwrite this)
        m_coordPanel->ClearPoints();
        m_coordPanel->AddPoint(x, y, *wxRED); 
    });
    
    rightSizer->Add(m_coordPanel, 1, wxEXPAND | wxALL, 10);

    parent->SetSizer(rightSizer); // Critical: Set the sizer for this specific panel
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

wxSizer* MainFrame::CreateControlBox(wxPanel* parent)
{
    wxStaticBoxSizer* boxSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Machine Control");
    wxWindow* innerParent = boxSizer->GetStaticBox();

    // 1. Step Size Row
    wxBoxSizer* stepRow = new wxBoxSizer(wxHORIZONTAL);
    stepRow->Add(new wxStaticText(innerParent, wxID_ANY, "Step:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    m_stepSizeCtrl = new wxSpinCtrlDouble(innerParent, wxID_ANY, "1.0", 
                                          wxDefaultPosition, wxSize(70, -1), 
                                          wxSP_ARROW_KEYS, 0.01, 100.0, 1.0, 0.1);
    
    stepRow->Add(m_stepSizeCtrl, 1, wxEXPAND | wxRIGHT, 5);
    stepRow->Add(new wxStaticText(innerParent, wxID_ANY, "mm"), 0, wxALIGN_CENTER_VERTICAL);
    boxSizer->Add(stepRow, 0, wxEXPAND | wxALL, 5);

    // 2. Feed Rate Row
    wxBoxSizer* feedRow = new wxBoxSizer(wxHORIZONTAL);
    feedRow->Add(new wxStaticText(innerParent, wxID_ANY, "Feed:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    // min: 1, max: 5000, initial: 1000
    m_feedRateCtrl = new wxSpinCtrl(innerParent, wxID_ANY, "1000", 
                                     wxDefaultPosition, wxSize(70, -1), 
                                     wxSP_ARROW_KEYS, 1, 5000, 1000);
    
    feedRow->Add(m_feedRateCtrl, 1, wxEXPAND | wxRIGHT, 5);
    feedRow->Add(new wxStaticText(innerParent, wxID_ANY, "mm/min"), 0, wxALIGN_CENTER_VERTICAL);
    boxSizer->Add(feedRow, 0, wxEXPAND | wxALL, 5);

    boxSizer->Add(new wxStaticLine(innerParent), 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
    // GridBagSizer allows precise placement for the directional "cross"
    wxGridBagSizer* jogGrid = new wxGridBagSizer(5, 5);

    // 2. Directional Buttons (Y+, Y-, X-, X+)
    wxButton* btnUp    = new wxButton(boxSizer->GetStaticBox(), ID_JOG_UP,    wxString::FromUTF8("▲"), wxDefaultPosition, wxSize(45, 45)); btnUp->SetToolTip(wxString::FromUTF8("Move Y+"));
    wxButton* btnDown  = new wxButton(boxSizer->GetStaticBox(), ID_JOG_DOWN,  wxString::FromUTF8("▼"), wxDefaultPosition, wxSize(45, 45)); btnDown->SetToolTip(wxString::FromUTF8("Move Y-"));
    wxButton* btnLeft  = new wxButton(boxSizer->GetStaticBox(), ID_JOG_LEFT,  wxString::FromUTF8("◀"), wxDefaultPosition, wxSize(45, 45)); btnLeft->SetToolTip(wxString::FromUTF8("Move X-"));
    wxButton* btnRight = new wxButton(boxSizer->GetStaticBox(), ID_JOG_RIGHT, wxString::FromUTF8("▶"), wxDefaultPosition, wxSize(45, 45)); btnRight->SetToolTip(wxString::FromUTF8("Move X+"));

    // Diagonal Buttons (Using ↖, ↗, ↙, ↘)
    wxButton* btnUpLeft    = new wxButton(innerParent, ID_JOG_UP_LEFT,    wxString::FromUTF8("↖"), wxDefaultPosition, wxSize(45, 45)); btnUpLeft->SetToolTip(wxString::FromUTF8("Move X- Y+"));
    wxButton* btnUpRight   = new wxButton(innerParent, ID_JOG_UP_RIGHT,   wxString::FromUTF8("↗"), wxDefaultPosition, wxSize(45, 45)); btnUpRight->SetToolTip(wxString::FromUTF8("Move X+ Y+"));
    wxButton* btnDownLeft  = new wxButton(innerParent, ID_JOG_DOWN_LEFT,  wxString::FromUTF8("↙"), wxDefaultPosition, wxSize(45, 45)); btnDownLeft->SetToolTip(wxString::FromUTF8("Move X- Y-"));
    wxButton* btnDownRight = new wxButton(innerParent, ID_JOG_DOWN_RIGHT, wxString::FromUTF8("↘"), wxDefaultPosition, wxSize(45, 45)); btnDownRight->SetToolTip(wxString::FromUTF8("Move X+ Y-"));

    // 3. Center Home Button with Icon
    wxBitmapButton* btnHome = new wxBitmapButton(boxSizer->GetStaticBox(), ID_HOME, 
        wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_BUTTON, wxSize(24, 24)));
    btnHome->SetToolTip("Home ($H)");

    // Position them in the corners
    jogGrid->Add(btnUpLeft,    wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnUp,        wxGBPosition(0, 1), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnUpRight,   wxGBPosition(0, 2), wxGBSpan(1, 1), wxALIGN_CENTER);

    jogGrid->Add(btnLeft,      wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnHome,      wxGBPosition(1, 1), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnRight,     wxGBPosition(1, 2), wxGBSpan(1, 1), wxALIGN_CENTER);

    jogGrid->Add(btnDownLeft,  wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnDown,      wxGBPosition(2, 1), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnDownRight, wxGBPosition(2, 2), wxGBSpan(1, 1), wxALIGN_CENTER);
    
    // 3. Control Buttons (Placed to the right or below)
    // Adding them to the right column of the same grid
    jogGrid->Add(new wxButton(boxSizer->GetStaticBox(), ID_UNLOCK, "Unlock ($X)"), wxGBPosition(0, 3), wxGBSpan(1, 1), wxEXPAND);
    jogGrid->Add(new wxButton(boxSizer->GetStaticBox(), ID_PAUSE, "Hold (!)"),     wxGBPosition(1, 3), wxGBSpan(1, 1), wxEXPAND);
    jogGrid->Add(new wxButton(boxSizer->GetStaticBox(), ID_RESUME, "Resume (~)"),  wxGBPosition(2, 3), wxGBSpan(1, 1), wxEXPAND);

    // 4. Emergency Stop (Full Width across all columns)
    wxButton* stopBtn = new wxButton(boxSizer->GetStaticBox(), ID_RESET, "SOFT RESET (CTRL+X)");
    stopBtn->SetBackgroundColour(wxColour(200, 50, 50)); 
    stopBtn->SetForegroundColour(*wxWHITE);
    stopBtn->SetFont(stopBtn->GetFont().Bold());

    // Add elements to the main vertical boxSizer
    boxSizer->Add(jogGrid, 0, wxALIGN_CENTER | wxALL, 10);
    boxSizer->Add(stopBtn, 0, wxEXPAND | wxALL, 5);

    return boxSizer;
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

        bool isSetting = m_grbl->ParseSetting(line);

        // 2. ONLY update the window if it is actually open
        if (isSetting && m_configDlg) {
            m_configDlg->ReloadGrid(); 
        }
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

void MainFrame::SetPreviewRegion(double x, double y, double width, double height)
{
    m_currentPreviewRegion = {x, y, width, height};
    if(m_coordPanel)
        m_coordPanel->DrawPreviewRegion(x, y, width, height);
}

void MainFrame::OnOpenSettings(wxCommandEvent &event)
{
    // if (!m_grbl->IsConnected()) {
    //     wxMessageBox("Please connect to the machine first.", "Error", wxICON_ERROR);
    //     return;
    // }

    // Create the dialog
    GrblConfigDialog dlg(this, m_grbl.get());
    
    // Register it so we can feed it data
    m_configDlg = &dlg;
    
    // Show it (Blocking / Modal)
    dlg.ShowModal();
    
    // Cleanup after it closes
    m_configDlg = nullptr;
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
        std::string portStd = selectedPort.ToStdString(); 

        m_connectBtn->SetLabel("Connecting...");
        m_connectBtn->Disable(); 
        m_portCombo->Enable(false);

        std::thread([this, portStd]() {
            

            bool success = m_grbl->Connect(portStd);
            
            wxTheApp->CallAfter([this, success, portStd]() {
                
                m_connectBtn->Enable();

                if (success) {
                    m_connectBtn->SetLabel("Disconnect");
                    wxLogMessage("Successfully connected to %s", portStd);
                } else {
                    m_connectBtn->SetLabel("Connect");
                    m_portCombo->Enable(true);
                    wxLogError("Failed to open port %s", portStd);
                }
            });

        }).detach();
    }
}

void MainFrame::OnSend(wxCommandEvent& event) {
    if (!m_grbl->IsConnected()) return;

    wxString cmd = m_cmdInput->GetValue();
    if (cmd.IsEmpty()) return;

    wxLogMessage("TX: %s", cmd);
    m_grbl->SendCommand(cmd.ToStdString());
    m_cmdInput->Clear();
}

void MainFrame::OnGoTo(wxCommandEvent& event) {
    if (!m_grbl || !m_grbl->IsConnected()) return;

    std::optional<double> xTarget;
    std::optional<double> yTarget;
    double val;

    m_coordPanel->DrawPreviewRegion(0, 0, 100, 100); // Example: Show a preview region (you can customize this)

    // Try to parse X input
    if (!m_xInput->GetValue().IsEmpty() && m_xInput->GetValue().ToDouble(&val)) {
        xTarget = val;
    }

    // Try to parse Y input
    if (!m_yInput->GetValue().IsEmpty() && m_yInput->GetValue().ToDouble(&val)) {
        yTarget = val;
    }

    // Only send if at least one axis was validly filled
    if (xTarget || yTarget) {
        m_grbl->MoveTo(xTarget, yTarget);
        wxLogMessage("Moving to absolute position...");
    } else {
        wxLogWarning("Please enter a valid coordinate for X or Y.");
    }
}

void MainFrame::OnHome(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendCommand(Grbl::Home); 
        wxLogMessage("Sent Homing Command ($H)");
    }
}

void MainFrame::OnUnlock(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendCommand(Grbl::Unlock);
        wxLogMessage("Sent Unlock Command ($X)");
    }
}

void MainFrame::OnPause(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::FeedHold); 
        wxLogMessage("Sent Feed Hold (!)");
    }
}

void MainFrame::OnResume(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::CycleStart); 
        wxLogMessage("Sent Cycle Start (~)");
    }
}

void MainFrame::OnReset(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::SoftReset);
        wxLogMessage("Soft Reset Sent!");
    }
}

void MainFrame::OnJog(wxCommandEvent& event) {
    if(!m_grbl || !m_grbl->IsConnected()) {
        wxLogWarning("Cannot jog: Not connected to GRBL.");
        return;
    }

    // Get values from UI controls
    double dist = m_stepSizeCtrl->GetValue();
    double feed = m_feedRateCtrl->GetValue();

    double moveX = 0.0;
    double moveY = 0.0;

    int id = event.GetId();

    // 1. Calculate X movement
    if (id == ID_JOG_LEFT || id == ID_JOG_UP_LEFT || id == ID_JOG_DOWN_LEFT) {
        moveX = -dist;
    } 
    else if (id == ID_JOG_RIGHT || id == ID_JOG_UP_RIGHT || id == ID_JOG_DOWN_RIGHT) {
        moveX = dist;
    }

    // 2. Calculate Y movement
    if (id == ID_JOG_UP || id == ID_JOG_UP_LEFT || id == ID_JOG_UP_RIGHT) {
        moveY = dist;
    } 
    else if (id == ID_JOG_DOWN || id == ID_JOG_DOWN_LEFT || id == ID_JOG_DOWN_RIGHT) {
        moveY = -dist;
    }

    // 3. Delegate to the controller
    m_grbl->JogTo(moveX, moveY, feed);
    
    // Optional: Log it for debugging
    wxLogMessage("Jog requested: X=%.3f, Y=%.3f (F%.0f)", moveX, moveY, feed);
}
