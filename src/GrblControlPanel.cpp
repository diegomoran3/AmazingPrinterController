#include "GrblControlPanel.hpp"

wxBEGIN_EVENT_TABLE(GrblControlPanel, wxPanel)
    EVT_BUTTON(GrblControlPanel::ID_GOTO,           GrblControlPanel::OnGoTo)
    EVT_BUTTON(GrblControlPanel::ID_HOME,            GrblControlPanel::OnHome)
    EVT_BUTTON(GrblControlPanel::ID_UNLOCK,          GrblControlPanel::OnUnlock)
    EVT_BUTTON(GrblControlPanel::ID_PAUSE,           GrblControlPanel::OnPause)
    EVT_BUTTON(GrblControlPanel::ID_RESUME,          GrblControlPanel::OnResume)
    EVT_BUTTON(GrblControlPanel::ID_RESET,           GrblControlPanel::OnReset)
    EVT_BUTTON(GrblControlPanel::ID_JOG_UP,          GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_DOWN,        GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_LEFT,        GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_RIGHT,       GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_UP_LEFT,     GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_UP_RIGHT,    GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_DOWN_LEFT,   GrblControlPanel::OnJog)
    EVT_BUTTON(GrblControlPanel::ID_JOG_DOWN_RIGHT,  GrblControlPanel::OnJog)
wxEND_EVENT_TABLE()

GrblControlPanel::GrblControlPanel(wxWindow* parent, std::shared_ptr<GrblController> grbl, JogSettings* settings)
    : wxPanel(parent, wxID_ANY),
      m_grbl(grbl),
      m_settings(settings)
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(CreateControlBox(this), 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(CreateMotionBox(this),  0, wxEXPAND | wxALL, 5);

    // Initialize controls from saved settings
    m_stepSizeCtrl->SetValue(m_settings->stepSize);
    m_feedRateCtrl->SetValue(m_settings->feedRate);

    // Bind change events to write back into settings
    m_stepSizeCtrl->Bind(wxEVT_SPINCTRLDOUBLE, [this](wxSpinDoubleEvent&) {
        m_settings->stepSize = m_stepSizeCtrl->GetValue();
    });
    m_feedRateCtrl->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&) {
        m_settings->feedRate = m_feedRateCtrl->GetValue();
    });

    SetSizer(mainSizer);
    Layout();
}

double GrblControlPanel::GetStepSize() const
{
    return m_stepSizeCtrl->GetValue();
}

double GrblControlPanel::GetFeedRate() const
{
    return m_feedRateCtrl->GetValue();
}

wxSizer* GrblControlPanel::CreateControlBox(wxWindow* parent)
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

    m_feedRateCtrl = new wxSpinCtrl(innerParent, wxID_ANY, "1000",
                                     wxDefaultPosition, wxSize(70, -1),
                                     wxSP_ARROW_KEYS, 1, 6000, 1);

    feedRow->Add(m_feedRateCtrl, 1, wxEXPAND | wxRIGHT, 5);
    feedRow->Add(new wxStaticText(innerParent, wxID_ANY, "mm/min"), 0, wxALIGN_CENTER_VERTICAL);
    boxSizer->Add(feedRow, 0, wxEXPAND | wxALL, 5);

    boxSizer->Add(new wxStaticLine(innerParent), 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);

    // GridBagSizer for the directional jog "cross"
    wxGridBagSizer* jogGrid = new wxGridBagSizer(5, 5);

    // Directional Buttons
    wxButton* btnUp    = new wxButton(innerParent, ID_JOG_UP,    wxString::FromUTF8("▲"), wxDefaultPosition, wxSize(45, 45)); btnUp->SetToolTip(wxString::FromUTF8("Move Y+"));
    wxButton* btnDown  = new wxButton(innerParent, ID_JOG_DOWN,  wxString::FromUTF8("▼"), wxDefaultPosition, wxSize(45, 45)); btnDown->SetToolTip(wxString::FromUTF8("Move Y-"));
    wxButton* btnLeft  = new wxButton(innerParent, ID_JOG_LEFT,  wxString::FromUTF8("◀"), wxDefaultPosition, wxSize(45, 45)); btnLeft->SetToolTip(wxString::FromUTF8("Move X-"));
    wxButton* btnRight = new wxButton(innerParent, ID_JOG_RIGHT, wxString::FromUTF8("▶"), wxDefaultPosition, wxSize(45, 45)); btnRight->SetToolTip(wxString::FromUTF8("Move X+"));

    // Diagonal Buttons
    wxButton* btnUpLeft    = new wxButton(innerParent, ID_JOG_UP_LEFT,    wxString::FromUTF8("↖"), wxDefaultPosition, wxSize(45, 45)); btnUpLeft->SetToolTip(wxString::FromUTF8("Move X- Y+"));
    wxButton* btnUpRight   = new wxButton(innerParent, ID_JOG_UP_RIGHT,   wxString::FromUTF8("↗"), wxDefaultPosition, wxSize(45, 45)); btnUpRight->SetToolTip(wxString::FromUTF8("Move X+ Y+"));
    wxButton* btnDownLeft  = new wxButton(innerParent, ID_JOG_DOWN_LEFT,  wxString::FromUTF8("↙"), wxDefaultPosition, wxSize(45, 45)); btnDownLeft->SetToolTip(wxString::FromUTF8("Move X- Y-"));
    wxButton* btnDownRight = new wxButton(innerParent, ID_JOG_DOWN_RIGHT, wxString::FromUTF8("↘"), wxDefaultPosition, wxSize(45, 45)); btnDownRight->SetToolTip(wxString::FromUTF8("Move X+ Y-"));

    // Center Home Button with Icon
    wxBitmapButton* btnHome = new wxBitmapButton(innerParent, ID_HOME,
        wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_BUTTON, wxSize(24, 24)));
    btnHome->SetToolTip("Home ($H)");

    // Position them in the grid
    jogGrid->Add(btnUpLeft,    wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnUp,        wxGBPosition(0, 1), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnUpRight,   wxGBPosition(0, 2), wxGBSpan(1, 1), wxALIGN_CENTER);

    jogGrid->Add(btnLeft,      wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnHome,      wxGBPosition(1, 1), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnRight,     wxGBPosition(1, 2), wxGBSpan(1, 1), wxALIGN_CENTER);

    jogGrid->Add(btnDownLeft,  wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnDown,      wxGBPosition(2, 1), wxGBSpan(1, 1), wxALIGN_CENTER);
    jogGrid->Add(btnDownRight, wxGBPosition(2, 2), wxGBSpan(1, 1), wxALIGN_CENTER);

    // Control Buttons to the right
    jogGrid->Add(new wxButton(innerParent, ID_UNLOCK, "Unlock ($X)"), wxGBPosition(0, 3), wxGBSpan(1, 1), wxEXPAND);
    jogGrid->Add(new wxButton(innerParent, ID_PAUSE,  "Hold (!)"),    wxGBPosition(1, 3), wxGBSpan(1, 1), wxEXPAND);
    jogGrid->Add(new wxButton(innerParent, ID_RESUME, "Resume (~)"),  wxGBPosition(2, 3), wxGBSpan(1, 1), wxEXPAND);

    // Emergency Stop (Full Width)
    wxButton* stopBtn = new wxButton(innerParent, ID_RESET, "SOFT RESET (CTRL+X)");
    stopBtn->SetBackgroundColour(wxColour(200, 50, 50));
    stopBtn->SetForegroundColour(*wxWHITE);
    stopBtn->SetFont(stopBtn->GetFont().Bold());

    boxSizer->Add(jogGrid, 0, wxALIGN_CENTER | wxALL, 10);
    boxSizer->Add(stopBtn, 0, wxEXPAND | wxALL, 5);

    return boxSizer;
}

wxSizer* GrblControlPanel::CreateMotionBox(wxWindow* parent)
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

// --- Event Handlers ---

void GrblControlPanel::OnGoTo(wxCommandEvent& event)
{
    if (!m_grbl || !m_grbl->IsConnected()) return;

    std::optional<double> xTarget;
    std::optional<double> yTarget;
    double val;

    if (!m_xInput->GetValue().IsEmpty() && m_xInput->GetValue().ToDouble(&val)) {
        xTarget = val;
    }

    if (!m_yInput->GetValue().IsEmpty() && m_yInput->GetValue().ToDouble(&val)) {
        yTarget = val;
    }

    if (xTarget || yTarget) {
        m_grbl->MoveTo(xTarget, yTarget);
        wxLogMessage("Moving to absolute position...");
    } else {
        wxLogWarning("Please enter a valid coordinate for X or Y.");
    }
}

void GrblControlPanel::OnHome(wxCommandEvent& event)
{
    if (m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendCommand(Grbl::Home);
        wxLogMessage("Sent Homing Command ($H)");
    }
}

void GrblControlPanel::OnUnlock(wxCommandEvent& event)
{
    if (m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendCommand(Grbl::Unlock);
        wxLogMessage("Sent Unlock Command ($X)");
    }
}

void GrblControlPanel::OnPause(wxCommandEvent& event)
{
    if (m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::FeedHold);
        wxLogMessage("Sent Feed Hold (!)");
    }
}

void GrblControlPanel::OnResume(wxCommandEvent& event)
{
    if (m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::CycleStart);
        wxLogMessage("Sent Cycle Start (~)");
    }
}

void GrblControlPanel::OnReset(wxCommandEvent& event)
{
    if (m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::SoftReset);
        wxLogMessage("Soft Reset Sent!");
    }
}

void GrblControlPanel::OnJog(wxCommandEvent& event)
{
    if (!m_grbl || !m_grbl->IsConnected()) {
        wxLogWarning("Cannot jog: Not connected to GRBL.");
        return;
    }

    double dist = m_stepSizeCtrl->GetValue();
    double feed = m_feedRateCtrl->GetValue();

    double moveX = 0.0;
    double moveY = 0.0;

    int id = event.GetId();

    // Calculate X movement
    if (id == ID_JOG_LEFT || id == ID_JOG_UP_LEFT || id == ID_JOG_DOWN_LEFT) {
        moveX = -dist;
    }
    else if (id == ID_JOG_RIGHT || id == ID_JOG_UP_RIGHT || id == ID_JOG_DOWN_RIGHT) {
        moveX = dist;
    }

    // Calculate Y movement
    if (id == ID_JOG_UP || id == ID_JOG_UP_LEFT || id == ID_JOG_UP_RIGHT) {
        moveY = dist;
    }
    else if (id == ID_JOG_DOWN || id == ID_JOG_DOWN_LEFT || id == ID_JOG_DOWN_RIGHT) {
        moveY = -dist;
    }

    m_grbl->JogTo(moveX, moveY, feed);
    wxLogMessage("Jog requested: X=%.3f, Y=%.3f (F%.0f)", moveX, moveY, feed);
}