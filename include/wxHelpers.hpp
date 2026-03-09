#pragma once

#include <wx/wx.h>
#include <wx/valnum.h>
#include <wx/valgen.h>

class wxHelpers
{
public:
    static void AddInputDouble(wxWindow* parent, wxFlexGridSizer* sizer, const wxString& label, 
                            wxTextCtrl*& ptr, double* dataPtr, 
                            std::function<void()> onUpdate, double minVal = 0.0) 
    {
        sizer->Add(new wxStaticText(parent, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL);
        
        wxFloatingPointValidator<double> val(2, dataPtr, wxNUM_VAL_DEFAULT);
        val.SetMin(minVal); 
        
        ptr = new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, val);
        sizer->Add(ptr, 1, wxEXPAND);

        if (onUpdate) {
            ptr->Bind(wxEVT_TEXT, [onUpdate](wxCommandEvent& event) {
                onUpdate();
                event.Skip();
            });
        }
    }

    static void AddInputInt(wxWindow* parent, wxFlexGridSizer* sizer, const wxString& label, 
                            wxTextCtrl*& ptr, int* dataPtr, 
                            std::function<void()> onChange, int minVal = 0) 
    {
        sizer->Add(new wxStaticText(parent, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        
        wxIntegerValidator<int> val(dataPtr, wxNUM_VAL_DEFAULT);
        val.SetMin(minVal);
        
        ptr = new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, val);
        sizer->Add(ptr, 1, wxEXPAND);

        if (onChange) {
            ptr->Bind(wxEVT_TEXT, [onChange](wxCommandEvent& event) {
                onChange();
                event.Skip();
            });
        }
    }

    static void AddCheckBox(wxWindow* parent, wxFlexGridSizer* sizer, const wxString& label, 
                            wxCheckBox*& ptr, bool* dataPtr, 
                            std::function<void()> onChange) 
    {
        ptr = new wxCheckBox(parent, wxID_ANY, label);
        ptr->SetValidator(wxGenericValidator(dataPtr));
        sizer->Add(new wxStaticText(parent, wxID_ANY, ""), 0); // Empty cell for alignment
        sizer->Add(ptr, 0, wxEXPAND);

        if (onChange) {
            ptr->Bind(wxEVT_CHECKBOX, [onChange](wxCommandEvent& event) {
                onChange();
                event.Skip();
            });
        }
    }
};