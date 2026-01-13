#ifndef COORDINATEPANEL_H
#define COORDINATEPANEL_H

#include <wx/wx.h>
#include <vector>

struct DataPoint {
    double x;
    double y;
    wxColour color;
};

class CoordinatePanel : public wxPanel {
public:
    CoordinatePanel(wxWindow* parent);

    void AddPoint(double x, double y, wxColour color = *wxRED);
    void ClearPoints();

private:
    void OnPaint(wxPaintEvent& evt);
    wxPoint CoordToScreen(double x, double y);

    double minX, maxX, minY, maxY;
    std::vector<DataPoint> points;
};

#endif
