#pragma once

struct GridPatternSettings {
    double startX = 0.0;
    double startY = 0.0;
    int rows = 1;
    int cols = 1;
    double stepX = 1.0;
    double stepY = 1.0;
    int speed = 6000;
    ScanDirection direction = ScanDirection::Horizontal;
    bool isZigzag = false;
};

struct AppSettings {
    public :
        bool showPreview = true;
        bool autoHomeOnConnect = true;
        int statusUpdateRefreshRateHz = 10;
        GridPatternSettings lastUsedPattern;
};