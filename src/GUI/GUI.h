#pragma once

#include "../GUI/Crosshair.h"

class GUI {
    friend class Engine;

public:
    Crosshair crosshair;

private:
    GUI();
    ~GUI() = default;
};