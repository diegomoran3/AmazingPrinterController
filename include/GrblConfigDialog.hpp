#pragma once

#include "GrblController.hpp"
#include <wx/wx.h>
#include <wx/grid.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

enum class GrblDataType {
    Integer,
    Float,
    Boolean,
    Mask
};

struct GrblSetting {
    int id;
    std::string name;
    GrblDataType type;
    std::string units;
    std::string description;
    std::string value;
};

class GrblConfigDialog : public wxDialog {
public:
    GrblConfigDialog(wxWindow* parent, std::shared_ptr<GrblController> controller);
    void ReloadGrid();

private:
    std::shared_ptr<GrblController> m_controller;
    wxGrid* m_grid;

    static const std::vector<GrblSetting>& GetGrblDefinitions();
    
    std::map<int, int> m_idToRowMap;

    void OnRefresh(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
