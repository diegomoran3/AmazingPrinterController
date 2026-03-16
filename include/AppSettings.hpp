#pragma once

#include "GrblController.hpp"

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
    bool isContinuous = false;
    double leadDistance = 0.0;
};

struct JogSettings {
    double stepSize = 10.0;
    int feedRate = 6000;
};

struct AppSettings {
    bool showPreview = true;

    bool autoHomeOnConnect = true;

    int statusPollRateHz = 5;        

    JogSettings jogSettings;

    GridPatternSettings lastUsedPattern;
};