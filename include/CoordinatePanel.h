#ifndef COORDINATEPANEL_H
#define COORDINATEPANEL_H

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <vector>

struct Point2D {
    double x, y;
    wxColour color;
};

class CoordinatePanel : public wxPanel {
public:
    CoordinatePanel(wxWindow* parent);
    void AddPoint(double x, double y, wxColour color = *wxRED);
    void ClearPoints();

private:
    std::vector<Point2D> points;
    double minX, maxX, minY, maxY;

    wxPoint CoordToScreen(double x, double y);
    void OnPaint(wxPaintEvent& evt);
};

#endif // COORDINATEPANEL_H
