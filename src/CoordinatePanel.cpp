#include "CoordinatePanel.h"
#include <cmath>
#include <wx/dcbuffer.h>

CoordinatePanel::CoordinatePanel(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Set the requested window size
    SetMinSize(wxSize(400, 380));
    SetSize(wxSize(400, 380));

    Bind(wxEVT_PAINT, &CoordinatePanel::OnPaint, this);
    
    // Set coordinate system to match requested range
    minX = 0.0; maxX = 400.0;
    minY = 0.0; maxY = 380.0;
}

wxPoint CoordinatePanel::CoordToScreen(double x, double y) {
    wxSize size = GetClientSize();
    
    // Margins to allow space for axis labels
    int marginLeft = 45;
    int marginBottom = 35;
    int marginRight = 20;
    int marginTop = 20;
    
    int drawableWidth = size.x - marginLeft - marginRight;
    int drawableHeight = size.y - marginBottom - marginTop;

    // Linear transformation: (value / range) * pixel_width
    int screenX = marginLeft + ((x - minX) / (maxX - minX)) * drawableWidth;
    int screenY = (size.y - marginBottom) - ((y - minY) / (maxY - minY)) * drawableHeight;
    
    return wxPoint(screenX, screenY);
}

void CoordinatePanel::AddPoint(double x, double y, wxColour color) {
    points.push_back({x, y, color});
    Refresh();
}

void CoordinatePanel::ClearPoints() {
    points.clear();
    Refresh();
}

void CoordinatePanel::OnPaint(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    // Draw Grid Lines every 50 units
    dc.SetPen(wxPen(wxColour(230, 230, 230), 1));
    int step = 50;

    for (int i = (int)minX; i <= (int)maxX; i += step) {
        dc.DrawLine(CoordToScreen(i, minY), CoordToScreen(i, maxY));
    }
    for (int i = (int)minY; i <= (int)maxY; i += step) {
        dc.DrawLine(CoordToScreen(minX, i), CoordToScreen(maxX, i));
    }
    
    // Draw Main Axes
    dc.SetPen(wxPen(*wxBLACK, 2));
    wxPoint origin = CoordToScreen(0, 0);
    wxPoint xEnd = CoordToScreen(maxX, 0);
    wxPoint yEnd = CoordToScreen(0, maxY);
    
    dc.DrawLine(origin, xEnd);
    dc.DrawLine(origin, yEnd);
    
    // Axis Labels
    dc.SetTextForeground(*wxBLACK);
    wxFont font = dc.GetFont();
    font.SetPointSize(7);
    dc.SetFont(font);
    
    // X-axis numbers
    for (int i = (int)minX; i <= (int)maxX; i += step) {
        wxPoint p = CoordToScreen(i, 0);
        dc.DrawText(wxString::Format("%d", i), p.x - 10, p.y + 5);
    }
    
    // Y-axis numbers
    for (int i = (int)minY; i <= (int)maxY; i += step) {
        if (i == 0) continue; 
        wxPoint p = CoordToScreen(0, i);
        dc.DrawText(wxString::Format("%d", i), p.x - 35, p.y - 7);
    }
    
    // Draw Points
    for (const auto& pt : points) {
        wxPoint screenPt = CoordToScreen(pt.x, pt.y);
        
        // Only draw if within visible range
        if (screenPt.x >= 0 && screenPt.y >= 0) {
            dc.SetBrush(wxBrush(pt.color));
            dc.SetPen(wxPen(pt.color, 1));
            dc.DrawCircle(screenPt, 3);
            
            // Optional: coordinate label
            dc.SetTextForeground(pt.color);
            dc.DrawText(wxString::Format("(%.0f,%.0f)", pt.x, pt.y), 
                       screenPt.x + 5, screenPt.y - 12);
        }
    }
}