#pragma once

#include "GrblController.hpp"
#include <atomic>
#include <functional>

struct ScanPoint {
    int row;
    int col;
    double x;
    double y;
};

struct ScanLine {
    double physicalStartX, physicalStartY;
    double physicalEndX, physicalEndY;

    std::vector<ScanPoint> points;
};

class ScanHandler {
    public:
        ScanHandler(std::shared_ptr<GrblController> grbl);

        void StartScanCycle(std::vector<ScanLine> scanLines, std::function<void(int, int, double, double)> onPointReached, int feedRate = 6000);

        void StartContinuousScanCycle(std::vector<ScanLine> scanLines, std::function<void(int, int, double, double)> onPointReached, int feedRate = 6000);

        std::vector<ScanLine> CreateScanLines(int rows, int cols, double startX, double startY, 
            double stepX, double stepY, bool direction, bool zigzag, double leadDistance = 0.0);
            
        void CancelScan();
        

    private:
        std::atomic<bool> m_shouldCancel{ false };
        std::shared_ptr<GrblController> m_grbl;

};