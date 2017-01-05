#ifndef DEFAULTSETTINGS_H
#define DEFAULTSETTINGS_H

#include <QString>

namespace ProgramDefaults {
    const int pollRate = 10;
    const bool leftStickSmoothEnabled = false;
    const bool rightStickSmoothEnabled = false;
    const int leftStickSmoothSize = 1;
    const int rightStickSmoothSize = 1;
    const int leftStickSmoothWeight = 100;
    const int rightStickSmoothWeight = 100;
    const QString leftStickDefaultCurve = "linear";
    const QString rightStickDefaultCurve = "linear";
    const int leftStickScale = 100;
    const int rightStickScale = 100;
    const int leftStickDeadZone = 0;
    const int rightStickDeadZone = 0;
    const int leftStickAntiDeadZone = 0;
    const int rightStickAntiDeadZone = 0;
    const int leftStickMaxZone = 100;
    const int rightStickMaxZone = 100;
    const double leftStickSens = 1.0;
    const double rightStickSens = 1.0;
    const int leftTriggerDeadZone = 0;
    const int rightTriggerDeadZone = 0;
}

#endif // DEFAULTSETTINGS_H

