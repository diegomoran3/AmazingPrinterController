#pragma once

#include "GrblController.hpp"
#include <atomic>
#include <functional>

class ScanHandler {
    public:
        ScanHandler(std::shared_ptr<GrblController> grbl);

        void StartScanCycle(double startX, double startY, 
            int rows, int cols, 
            double stepX, double stepY, 
            std::function<void(int, int, double, double)> onPointReached,
            ScanDirection direction,
            bool zigzag = false,
            double speed = 6000);

        void StartContinuousScanCycle(double startX, double startY, 
            int rows, int cols, 
            double stepX, double stepY, 
            std::function<void(int, int, double, double)> onPointReached, 
            ScanDirection direction, bool zigzag, double feedRate, double leadDistance);
            
        void CancelScan();
        

    private:
        std::atomic<bool> m_shouldCancel{ false };
        std::shared_ptr<GrblController> m_grbl;

};