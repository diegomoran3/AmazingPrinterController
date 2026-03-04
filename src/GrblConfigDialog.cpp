#include "GrblConfigDialog.h"

enum { ID_GRID_REFRESH = 100, ID_GRID_SAVE };

wxBEGIN_EVENT_TABLE(GrblConfigDialog, wxDialog)
    EVT_BUTTON(ID_GRID_REFRESH, GrblConfigDialog::OnRefresh)
    EVT_BUTTON(ID_GRID_SAVE, GrblConfigDialog::OnSave)
    EVT_BUTTON(wxID_CLOSE, GrblConfigDialog::OnClose)
wxEND_EVENT_TABLE()

GrblConfigDialog::GrblConfigDialog(wxWindow* parent, GrblController* controller)
    : wxDialog(parent, wxID_ANY, "GRBL Configuration", wxDefaultPosition, wxSize(500, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_controller(controller)
{
wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto& defs = GetGrblDefinitions();

    m_grid = new wxGrid(this, wxID_ANY);
    m_grid->CreateGrid(defs.size(), 5); // Added 5th column

    m_grid->SetColLabelValue(0, "ID");
    m_grid->SetColLabelValue(1, "Setting");
    m_grid->SetColLabelValue(2, "Value");
    m_grid->SetColLabelValue(3, "Units");
    m_grid->SetColLabelValue(4, "Description (Units)");

    for (size_t i = 0; i < defs.size(); ++i) {
        const auto& s = defs[i];
        m_idToRowMap[s.id] = i; // Map ID to Row index

        m_grid->SetCellValue(i, 0, "$" + std::to_string(s.id));
        m_grid->SetCellValue(i, 1, s.name);
        m_grid->SetCellValue(i, 3, s.units); // Set Type Text
        m_grid->SetCellValue(i, 4, s.description + " (" + s.units + ")");
        
        // Make metadata read-only
        m_grid->SetReadOnly(i, 0);
        m_grid->SetReadOnly(i, 1);
        m_grid->SetReadOnly(i, 3); // Type is informative, not editable
        m_grid->SetReadOnly(i, 4);

        // Apply specialized editors
        if (s.type == GrblDataType::Boolean) {
            m_grid->SetCellEditor(i, 3, new wxGridCellChoiceEditor(2, new wxString[2]{"False", "True"}, false));
        } else if (s.type == GrblDataType::Integer) {
            m_grid->SetCellEditor(i, 3, new wxGridCellNumberEditor());
        } else if (s.type == GrblDataType::Float) {
            m_grid->SetCellEditor(i, 3, new wxGridCellFloatEditor());
        }
    }

    m_grid->AutoSizeColumns();
    mainSizer->Add(m_grid, 1, wxEXPAND | wxALL, 10);

    // 2. Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, ID_GRID_REFRESH, "Read Settings ($$)"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, ID_GRID_SAVE, "Write Changes"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, "Close"), 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);

    SetSizer(mainSizer);
    
    // Auto-load settings on open
    m_controller->SendCommand("$$");
}

void GrblConfigDialog::ReloadGrid() {
    auto settings = m_controller->GetSettings();
    
    if (m_grid->GetNumberRows() > 0) m_grid->DeleteRows(0, m_grid->GetNumberRows());

    for (auto const& [id, val] : settings) {
        m_grid->AppendRows(1);
        int row = m_grid->GetNumberRows() - 1;
        m_grid->SetCellValue(row, 0, id);
        m_grid->SetCellValue(row, 1, val);
    }
}

const std::vector<GrblSetting> &GrblConfigDialog::GetGrblDefinitions()
{
    static const std::vector<GrblSetting> definitions = {
        {0, "Step pulse time", GrblDataType::Integer, "microseconds", "Sets time length per step. Minimum 3usec."},
        {1, "Step idle delay", GrblDataType::Integer, "milliseconds", "Sets a short hold delay when stopping. 255 keeps motors enabled."},
        {2, "Step pulse invert", GrblDataType::Mask, "mask", "Inverts the step signal (00000ZYX)."},
        {3, "Step direction invert", GrblDataType::Mask, "mask", "Inverts the direction signal (00000ZYX)."},
        {4, "Invert step enable pin", GrblDataType::Boolean, "boolean", "Inverts the stepper driver enable pin signal."},
        {5, "Invert limit pins", GrblDataType::Boolean, "boolean", "Inverts the all of the limit input pins."},
        {6, "Invert probe pin", GrblDataType::Boolean, "boolean", "Inverts the probe input pin signal."},
        {10, "Status report options", GrblDataType::Mask, "mask", "Alters data included in status reports."},
        {11, "Junction deviation", GrblDataType::Float, "millimeters", "Sets how fast Grbl travels through consecutive motions."},
        {12, "Arc tolerance", GrblDataType::Float, "millimeters", "Sets the G2 and G3 arc tracing accuracy."},
        {13, "Report in inches", GrblDataType::Boolean, "boolean", "Enables inch units for positions and rates."},
        {20, "Soft limits enable", GrblDataType::Boolean, "boolean", "Enables soft limits checks. Requires homing."},
        {21, "Hard limits enable", GrblDataType::Boolean, "boolean", "Immediately halts motion on switch trigger."},
        {22, "Homing cycle enable", GrblDataType::Boolean, "boolean", "Enables homing cycle. Requires limit switches."},
        {23, "Homing direction invert", GrblDataType::Mask, "mask", "Set axis bit to search in negative direction."},
        {24, "Homing locate feed rate", GrblDataType::Float, "mm/min", "Feed rate to slowly engage limit switch."},
        {25, "Homing search seek rate", GrblDataType::Float, "mm/min", "Seek rate to quickly find the limit switch."},
        {26, "Homing switch debounce delay", GrblDataType::Integer, "milliseconds", "Short delay to let a switch debounce."},
        {27, "Homing switch pull-off distance", GrblDataType::Float, "millimeters", "Retract distance after triggering switch."},
        {30, "Maximum spindle speed", GrblDataType::Float, "RPM", "Maximum spindle speed (100% PWM)."},
        {31, "Minimum spindle speed", GrblDataType::Float, "RPM", "Minimum spindle speed (0.4% PWM)."},
        {32, "Laser-mode enable", GrblDataType::Boolean, "boolean", "Enables laser mode to prevent motion halts on S changes."},
        {100, "X-axis travel resolution", GrblDataType::Float, "step/mm", "X-axis steps per millimeter."},
        {101, "Y-axis travel resolution", GrblDataType::Float, "step/mm", "Y-axis steps per millimeter."},
        {102, "Z-axis travel resolution", GrblDataType::Float, "step/mm", "Z-axis steps per millimeter."},
        {110, "X-axis maximum rate", GrblDataType::Float, "mm/min", "X-axis maximum G0 rate."},
        {111, "Y-axis maximum rate", GrblDataType::Float, "mm/min", "Y-axis maximum G0 rate."},
        {112, "Z-axis maximum rate", GrblDataType::Float, "mm/min", "Z-axis maximum G0 rate."},
        {120, "X-axis acceleration", GrblDataType::Float, "mm/sec^2", "X-axis acceleration for motion planning."},
        {121, "Y-axis acceleration", GrblDataType::Float, "mm/sec^2", "Y-axis acceleration for motion planning."},
        {122, "Z-axis acceleration", GrblDataType::Float, "mm/sec^2", "Z-axis acceleration for motion planning."},
        {130, "X-axis maximum travel", GrblDataType::Float, "millimeters", "Max X travel from homing switch."},
        {131, "Y-axis maximum travel", GrblDataType::Float, "millimeters", "Max Y travel from homing switch."},
        {132, "Z-axis maximum travel", GrblDataType::Float, "millimeters", "Max Z travel from homing switch."}
    };

    return definitions;
}

void GrblConfigDialog::OnRefresh(wxCommandEvent &event)
{
    if (m_controller->IsConnected());
        m_controller->SendCommand("$$");
}

void GrblConfigDialog::OnSave(wxCommandEvent& event) {
    // Iterate over rows and send updates
    for (int i = 0; i < m_grid->GetNumberRows(); ++i) {
        std::string id = m_grid->GetCellValue(i, 0).ToStdString();
        std::string val = m_grid->GetCellValue(i, 1).ToStdString();
        
        // Construct command: $100=250.000
        std::string cmd = id + "=" + val;
        m_controller->SendCommand(cmd);
    }
    wxMessageBox("Configuration Sent!", "Info", wxOK | wxICON_INFORMATION);
}

void GrblConfigDialog::OnClose(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}