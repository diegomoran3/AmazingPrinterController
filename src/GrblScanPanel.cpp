#include "GrblScanPanel.hpp"

// IDs for events
enum {
    ID_BTN_START = 1001,
    ID_BTN_CLOSE = 1002
};

wxBEGIN_EVENT_TABLE(GrblScanPanel, wxPanel)
    EVT_BUTTON(ID_BTN_START, GrblScanPanel::OnStart)
    EVT_TIMER(GrblScanPanel::PREVIEW_TIMER_ID, GrblScanPanel::OnPreviewTimerFired)
wxEND_EVENT_TABLE()

GrblScanPanel::GrblScanPanel(wxWindow* parent, std::shared_ptr<ScanHandler> controller, PreviewCallback onPreviewUpdate, GridPatternSettings* initialSettings)
    : wxPanel(parent, wxID_ANY),
      m_controller(controller),
      OnPreviewUpdate(onPreviewUpdate),
      m_settings(initialSettings),
      m_previewTimer(this, PREVIEW_TIMER_ID)
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* formSizer = new wxFlexGridSizer(0, 2, 5, 10); 
    formSizer->AddGrowableCol(1, 1);

    // 1. Create Fields
    wxHelpers::AddInputDouble(this, formSizer, "Start X:", m_txtStartX, &m_settings->startX, [this]() { this->OnUIChange(); }, -400);
    wxHelpers::AddInputDouble(this, formSizer, "Start Y:", m_txtStartY, &m_settings->startY, [this]() { this->OnUIChange(); }, -380);
    wxHelpers::AddInputInt(this, formSizer, "Rows:",    m_txtRows,   &m_settings->rows, [this]() { this->OnUIChange(); }, -1);
    wxHelpers::AddInputInt(this, formSizer, "Cols:",    m_txtCols,   &m_settings->cols, [this]() { this->OnUIChange(); }, 1);
    wxHelpers::AddInputDouble(this, formSizer, "Step X:",  m_txtStepX,  &m_settings->stepX, [this]() { this->OnUIChange(); }, 0.1);
    wxHelpers::AddInputDouble(this, formSizer, "Step Y:",  m_txtStepY,  &m_settings->stepY, [this]() { this->OnUIChange(); }, 0.1);
    wxHelpers::AddInputInt(this, formSizer, "Speed:",   m_txtSpeed,  &m_settings->speed, [this]() { this->OnUIChange(); }, 1);

    // 2. Direction
    wxString choices[] = { "Horizontal", "Vertical" };
    m_rbDirection = new wxRadioBox(this, wxID_ANY, "Direction", wxDefaultPosition, wxDefaultSize, 2, choices, 1, wxRA_SPECIFY_ROWS,
                                   wxGenericValidator((int*)&m_settings->direction));

    wxHelpers::AddCheckBox(this, formSizer, "Zigzag Mode (Snake Scan)", m_chkZigzag, &m_settings->isZigzag, [this]() { this->OnUIChange(); });
    wxHelpers::AddCheckBox(this, formSizer, "Continuous Scan", m_chkContinuous, &m_settings->isContinuous, [this]() { this->OnUIChange(); });
    wxHelpers::AddInputDouble(this, formSizer, "Lead Dist:", m_txtLeadDistance, &m_settings->leadDistance, [this]() { this->OnUIChange(); }, 0.0);

    m_rbDirection->Bind(wxEVT_RADIOBOX, [this](wxCommandEvent&) { this->OnUIChange(); });

    // 3. Buttons
    auto* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnStart = new wxButton(this, ID_BTN_START, "Start Scan");
    btnSizer->Add(m_btnStart, 1, wxRIGHT, 5);

    // Layout
    mainSizer->Add(formSizer, 0, wxALL | wxEXPAND, 15);
    mainSizer->Add(m_rbDirection, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 15);
    mainSizer->Add(btnSizer, 0, wxALL | wxALIGN_RIGHT, 15);

    SetSizer(mainSizer);
    SetSettings(*m_settings);

    Layout();
}

GrblScanPanel::~GrblScanPanel() {
    m_previewTimer.Stop();
    if (m_isScanning) {
        m_controller->CancelScan();
    }
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void GrblScanPanel::RebindValidators()
{
    // Re-point all validators to the current m_settings struct.
    // This is what makes SetSettings() work with just TransferDataToWindow().
    m_txtStartX->SetValidator(wxFloatingPointValidator<double>(3, &m_settings->startX));
    m_txtStartY->SetValidator(wxFloatingPointValidator<double>(3, &m_settings->startY));
    m_txtRows->SetValidator(wxIntegerValidator<int>(&m_settings->rows));
    m_txtCols->SetValidator(wxIntegerValidator<int>(&m_settings->cols));
    m_txtStepX->SetValidator(wxFloatingPointValidator<double>(3, &m_settings->stepX));
    m_txtStepY->SetValidator(wxFloatingPointValidator<double>(3, &m_settings->stepY));
    m_txtSpeed->SetValidator(wxIntegerValidator<int>(&m_settings->speed));
    m_txtLeadDistance->SetValidator(wxFloatingPointValidator<double>(3, &m_settings->leadDistance));
    m_chkZigzag->SetValidator(wxGenericValidator(&m_settings->isZigzag));
    m_chkContinuous->SetValidator(wxGenericValidator(&m_settings->isContinuous));
    m_rbDirection->SetValidator(wxGenericValidator((int*)&m_settings->direction));
}

void GrblScanPanel::ToggleControls(bool enable) {
    m_txtStartX->Enable(enable);
    m_txtStartY->Enable(enable);
    m_txtRows->Enable(enable);
    m_txtCols->Enable(enable);
    m_txtStepX->Enable(enable);
    m_txtStepY->Enable(enable);
    m_txtSpeed->Enable(enable);
    m_txtLeadDistance->Enable(enable);
    m_rbDirection->Enable(enable);
    m_chkZigzag->Enable(enable);
    m_chkContinuous->Enable(enable);

    if (enable) {
        m_btnStart->Enable(true);
    }
}

void GrblScanPanel::OnUIChange()
{
    if (this->TransferDataFromWindow()) {
        m_btnStart->Enable(true);
    
        m_previewTimer.StartOnce(PREVIEW_DELAY_MS);
    } else {
        m_btnStart->Enable(false);
        m_previewTimer.Stop();
    }
}

void GrblScanPanel::OnPreviewTimerFired(wxTimerEvent& event)
{
    if (!OnPreviewUpdate) return;

    auto scanLines = m_controller->CreateScanLines(
        m_settings->rows, m_settings->cols,
        m_settings->startX, m_settings->startY,
        m_settings->stepX, m_settings->stepY,
        m_settings->direction, m_settings->isZigzag,
        m_settings->leadDistance);
    
    OnPreviewUpdate(scanLines);
}

void GrblScanPanel::SetSettings(GridPatternSettings &pattern)
{
    m_settings = &pattern;

    m_chkContinuous->SetValue(m_settings->isContinuous);
    m_chkZigzag->SetValue(m_settings->isZigzag);
    m_rbDirection->SetSelection(m_settings->direction == ScanDirection::Horizontal ? 0 : 1);
    TransferDataToWindow();
}

void GrblScanPanel::RefreshPreview()
{
    if (!OnPreviewUpdate) return;

    auto scanLines = m_controller->CreateScanLines(
        m_settings->rows, m_settings->cols,
        m_settings->startX, m_settings->startY,
        m_settings->stepX, m_settings->stepY,
        m_settings->direction, m_settings->isZigzag,
        m_settings->leadDistance);

    OnPreviewUpdate(scanLines);
}

void GrblScanPanel::OnStart(wxCommandEvent &event)
{
    if (m_isScanning) {
        m_controller->CancelScan();
        m_btnStart->Disable();
        m_btnStart->SetLabel("Stopping...");
        return;
    }

    try {

        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
        
        m_isScanning = true;
        ToggleControls(false); // Disable inputs
        
        m_btnStart->Enable(true); 
        m_btnStart->SetLabel("STOP SCAN"); // Change label
        
        Layout();

        m_workerThread = std::thread([=, this]() {

            auto scanLines = m_controller->CreateScanLines(m_settings->rows, m_settings->cols, m_settings->startX, m_settings->startY, 
                m_settings->stepX, m_settings->stepY, m_settings->direction, m_settings->isZigzag, m_settings->leadDistance);
            
            auto pointCallback = [](int r, int c, double x, double y) {
                //Jonathan: TODO for point reached callback
            };

            if (m_settings->isContinuous) {
                m_controller->StartContinuousScanCycle(scanLines, pointCallback, m_settings->speed);
            } else {
                m_controller->StartScanCycle(scanLines, pointCallback, m_settings->speed);
            }

            // Update UI when finished (or cancelled)
            this->CallAfter([this]() {
                m_isScanning = false;
                
                // Restore UI state
                m_btnStart->SetLabel("Start Scan");
                ToggleControls(true); // Re-enables inputs and buttons
                Layout();
                
                wxMessageBox("Scan cycle ended.", "Info");
            });
        });

    } catch (...) {
        wxMessageBox("Invalid input", "Error");
    }
}