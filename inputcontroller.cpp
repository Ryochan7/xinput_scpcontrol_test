#include <QDebug>
#include <QtMath>

#include "inputcontroller.h"
#include "defaultsettings.h"

const int InputController::AXIS_MIN = -32768;
const int InputController::AXIS_MAX = 32767;
const int InputController::MAXSMOOTHINGSIZE = 30;

InputController::InputController(ScpBusDevice *busDevice, QObject *parent) :
    QObject(parent)
{
    this->busDevice = busDevice;
    xinputIndex = 1;
    memset(&buttons, 0, sizeof(buttons));
    memset(&axes, 0, sizeof(axes));
    memset(&smoothingEnabled, 0, sizeof(smoothingEnabled));
    memset(&smoothingWeights, 0, sizeof(smoothingWeights));
    memset(&smoothingSizes, 0, sizeof(smoothingSizes));
    memset(&axisScales, 0, sizeof(axisScales));
    memset(&axisDeadZones, 0, sizeof(axisDeadZones));
    memset(&axisAntiDeadZones, 0, sizeof(axisAntiDeadZones));
    memset(&axisMaxZones, 0, sizeof(axisMaxZones));
    memset(&axisSens, 0, sizeof(axisSens));

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        smoothingWeights[i] = 1.0;
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        smoothingSizes[i] = 1;
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        smoothingBuffer[i].prepend(0);
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        axisScales[i] = 1.0;
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        axisMaxZones[i] = 100;
    }

    hat = 0;
}

void InputController::setBusDevice(ScpBusDevice *busDevice)
{
    this->busDevice = busDevice;
}

void InputController::buttonEvent(int buttonIndex, bool value)
{
    if (buttonIndex >= 0 && buttonIndex < static_cast<int>(MAXBUTTONS))
    {
        buttons[buttonIndex] = value;
    }
}

void InputController::axisEvent(int axisIndex, int value)
{
    if (axisIndex >= 0 && axisIndex < static_cast<int>(MAXAXES) &&
        value >= AXIS_MIN && value <= AXIS_MAX)
    {
        axes[axisIndex] = value;
    }
}

void InputController::hatEvent(int value)
{
    if (value >= 0 && value <= (0x08 | 0x04))
    {
        if (value == 0)
        {
            hat = 0;
        }
        else if (hat != value)
        {
            int changed = hat ^ value;
            //int removed = changed & hat;
            //int added = changed & value;

            hat ^= changed;
            //hat |= added;
            //hat |= value;
        }

    }
}

void InputController::generateXinputReport(byte *&report)
{
    report = new byte[20];
    memset(report, 0x00, sizeof(*report) * 20);

    report[0] = 0x00;                                 // Message type (input report)
    report[1] = 0x14;                                 // Message size (20 bytes)

    unsigned int buttonsTest = NONE;
    /*buttonsTest |= (hat & 0x01) > 0 ? DPAD_UP : 0;
    buttonsTest |= (hat & 0x02) > 0 ? DPAD_RIGHT : 0;
    buttonsTest |= (hat & 0x04) > 0 ? DPAD_DOWN : 0;
    buttonsTest |= (hat & 0x08) > 0 ? DPAD_LEFT : 0;
    */

    buttonsTest |= buttons[BUTTON_DPADUP] ? DPAD_UP : 0;
    buttonsTest |= buttons[BUTTTON_DPADRIGHT] ? DPAD_RIGHT : 0;
    buttonsTest |= buttons[BUTTON_DPADDOWN] ? DPAD_DOWN : 0;
    buttonsTest |= buttons[BUTTTON_DPADLEFT] ? DPAD_LEFT : 0;


    buttonsTest |= buttons[BUTTON_START] ? START : 0;
    buttonsTest |= buttons[BUTTON_BACK] ? BACK : 0;
    buttonsTest |= buttons[BUTTON_GUIDE] ? GUIDE : 0;
    buttonsTest |= buttons[BUTTON_LEFTSTICK] ? LEFTSTICK : 0;
    buttonsTest |= buttons[BUTTON_RIGHTSTICK] ? RIGHTSTICK : 0;
    buttonsTest |= buttons[BUTTON_LEFTSHOULDER] ? LEFTBUMPER : 0;
    buttonsTest |= buttons[BUTTON_RIGHTSHOULDER] ? RIGHTBUMPER : 0;

    buttonsTest |= buttons[BUTTON_A] ? A : 0;
    buttonsTest |= buttons[BUTTON_B] ? B : 0;
    buttonsTest |= buttons[BUTTON_X] ? X : 0;
    buttonsTest |= buttons[BUTTON_Y] ? Y : 0;

    report[2] = (byte)(buttonsTest >> 0 & 0xFF);  // Buttons low
    report[3] = (byte)(buttonsTest >> 8 & 0xFF);  // Buttons high

    //report[4] = buttons[L2] ? 255 : 0;           // Left trigger
    //report[5] = buttons[R2] ? 255 : 0;           // Right trigger

    //qDebug() << "LEFT TRIGGER: " << axes[4] << ((axes[4] - 2000) / static_cast<double>(AXIS_MAX - 2000)) * 255;
    report[4] = (axes[4] > 3000) ? static_cast<byte>(((axes[4] - 3000) / static_cast<double>(AXIS_MAX - 3000)) * 255) : 0;        // Left trigger
    report[5] = (axes[5] > 3000) ? static_cast<byte>(((axes[5] - 3000) / static_cast<double>(AXIS_MAX - 3000)) * 255) : 0;        // Right trigger
    //qDebug() << "REPORT 5: " << report[5];

    int currentXAxis = 0;
    int currentYAxis = 1;

    int tempLx = computeSmoothedValue(0);
    tempLx = qBound(AXIS_MIN, qFloor(tempLx * axisScales[0]), AXIS_MAX);
    int tempLy = computeSmoothedValue(1);
    tempLy = qBound(AXIS_MIN, qFloor(tempLy * axisScales[1]), AXIS_MAX);
    calculateStickValuesAfterDead(0, 1, tempLx, tempLy, tempLx, tempLy);

    //tempLx = calculateAxisValueAfterDead(0, tempLx);
    //tempLy = calculateAxisValueAfterDead(1, tempLy);
    tempLx = getCurvedAxisValue(axisCurves[0], tempLx, 0);
    short lxAxis = static_cast<short>(qBound(AXIS_MIN, tempLx, AXIS_MAX));
    //short lxAxis = static_cast<short>(qBound(-32768, computeSmoothedValue(0), 32767));
    report[6] = static_cast<byte>(lxAxis & 0xFF);           // Left stick X-axis low
    report[7] = static_cast<byte>(lxAxis >> 8 & 0xFF);      // Left stick X-axis high

    tempLy = getCurvedAxisValue(axisCurves[1], tempLy, 1);
    //short lyAxis = static_cast<short>(qBound(AXIS_MIN, -tempLy, AXIS_MAX));
    short lyAxis = static_cast<short>(qBound(AXIS_MIN, calculateFlippedAxisValue(tempLy), AXIS_MAX));
    //short lyAxis = static_cast<short>(qBound(-32768, -computeSmoothedValue(1), 32767));
    report[8] = static_cast<byte>(lyAxis & 0xFF);           // Left stick Y-axis low
    report[9] = static_cast<byte>(lyAxis >> 8 & 0xFF);      // Left stick Y-axis high

    //qDebug() << "AXIS 1: " << axes[1];
    //qDebug() << "AXIS 2: " << axes[2];

    currentXAxis = 2; // SDL GC
    currentYAxis = 3; // SDL GC

    //currentXAxis = 3; // Twin
    //currentYAxis = 2; // Twin
    //currentXAxis = 2; // MP-8866
    //currentYAxis = 3; // MP-8866

    int tempRx = computeSmoothedValue(currentXAxis);
    tempRx = qBound(AXIS_MIN, qFloor(tempRx * axisScales[currentXAxis]), AXIS_MAX);
    int tempRy = computeSmoothedValue(currentYAxis);
    tempRy = qBound(AXIS_MIN, qFloor(tempRy * axisScales[currentYAxis]), AXIS_MAX);
    int tempOutRx = tempRx;
    int tempOutRy = tempRy;
    calculateStickValuesAfterDead(currentXAxis, currentYAxis, tempRx, tempRy, tempOutRx, tempOutRy);

    //qDebug() << "AXIS YS: " << tempRx << " | " << tempOutRx;

    tempRx = tempOutRx;
    tempRy = tempOutRy;

    //tempRx = calculateAxisValueAfterDead(currentXAxis, tempRx);
    //tempRy = calculateAxisValueAfterDead(currentYAxis, tempRy);
    tempRx = getCurvedAxisValue(axisCurves[currentXAxis], tempRx, currentXAxis);
    short rxAxis = static_cast<short>(qBound(AXIS_MIN, tempRx, AXIS_MAX));
    //short rxAxis = static_cast<short>(qBound(-32768, computeSmoothedValue(3), 32767)); // Twin
    //short rxAxis = static_cast<short>(qBound(-32767, axes[2], 32767));
    report[10] = static_cast<byte>(rxAxis & 0xFF);          // Right stick X-axis low
    report[11] = static_cast<byte>(rxAxis >> 8 & 0xFF);     // Right stick X-axis high

    tempRy = getCurvedAxisValue(axisCurves[currentYAxis], tempRy, currentYAxis);
    //short ryAxis = static_cast<short>(qBound(AXIS_MIN, -tempRy, AXIS_MAX));
    //qDebug() << "ORIG: " << tempRy << " | " << "NOW: " << calculateFlippedAxisValue(tempRy);
    short ryAxis = static_cast<short>(qBound(AXIS_MIN, calculateFlippedAxisValue(tempRy), AXIS_MAX));
    //short ryAxis = static_cast<short>(qBound(-32768, -computeSmoothedValue(2), 32767)); // Twin
    //short ryAxis = static_cast<short>(qBound(-32767, -axes[3], 32767));

    //qDebug() << "INITIAL: " << axes[currentXAxis] << " | " << "FINAL: " << tempRx;
    //qDebug() << "INITIAL: " << axes[currentYAxis] << " | " << "FINAL: " << tempRy;

    report[12] = static_cast<byte>(ryAxis & 0xFF);          // Right stick Y-axis low
    report[13] = static_cast<byte>(ryAxis >> 8 & 0xFF);     // Right stick Y-axis high
}

void InputController::outputReport()
{
    byte *controllerreport = 0;
    //byte finalreport[28];
    //memset(&finalreport, 0, sizeof(finalreport));
    //busDevice->generateReport(controllerreport);
    generateXinputReport(controllerreport);

    byte outputReport[8];
    memset(&outputReport, 0, sizeof(outputReport));

    busDevice->report(static_cast<unsigned int>(xinputIndex), controllerreport, outputReport);

    if (controllerreport)
    {
        delete [] controllerreport;
        controllerreport = 0;
    }
}

int InputController::computeSmoothedValue(int axis)
{
    int adjustedValue = axes[axis];

    if (smoothingEnabled[axis])
    {
        int smoothingSize = smoothingSizes[axis];
        if (smoothingBuffer[axis].size() >= smoothingSize)
        {
            smoothingBuffer[axis].removeLast();
        }

        smoothingBuffer[axis].prepend(axes[axis]);
        QListIterator<int> iter(smoothingBuffer[axis]);
        double currentWeight = 1.0;
        //double weightModifier = 1.0;
        double weightModifier = smoothingWeights[axis];
        double finalWeight = 0.0;
        adjustedValue = 0;

        while (iter.hasNext())
        {
            int temp = iter.next();
            adjustedValue += temp * currentWeight;
            finalWeight += currentWeight;
            currentWeight *= weightModifier;
        }

        adjustedValue = qFloor(adjustedValue / finalWeight);
    }

    return adjustedValue;
}

void InputController::changeSmoothingStatus(int axis, bool enabled)
{
    if (axis >= 0 && axis < MAXAXES)
    {
        smoothingBuffer[axis].clear();
        smoothingEnabled[axis] = enabled;

        if (enabled)
        {
            int size = smoothingSizes[axis];
            for (int i=0; i < size; i++)
            {
                smoothingBuffer[axis].prepend(0);
            }
        }
    }
}

void InputController::changeSmoothingSize(int axis, int size)
{
    if (axis >= 0 && axis < MAXAXES &&
        size >= 0 && size <= MAXSMOOTHINGSIZE)
    {
        smoothingSizes[axis] = size;
        smoothingBuffer[axis].clear();
        for (int i=0; i < size; i++)
        {
            smoothingBuffer[axis].prepend(0);
        }
    }
}

void InputController::changeSmoothingWeight(int axis, double weight)
{
    if (axis >= 0 && axis < MAXAXES &&
        weight >= 0.0 && weight <= 1.0)
    {
        smoothingWeights[axis] = weight;
    }
}

void InputController::readSettings(QSettings *settings)
{
    bool leftStickSmooth = settings->value("leftStickSmoothEnabled",
                                           ProgramDefaults::leftStickSmoothEnabled).toBool();
    if (leftStickSmooth)
    {
        changeSmoothingStatus(0, true);
        changeSmoothingStatus(1, true);
    }

    bool rightStickSmooth = settings->value("rightStickSmoothEnabled",
                                            ProgramDefaults::rightStickSmoothEnabled).toBool();
    if (rightStickSmooth)
    {
        changeSmoothingStatus(2, true);
        changeSmoothingStatus(3, true);
    }

    int leftStickSmoothSize = settings->value("leftStickSmoothSize",
                                              ProgramDefaults::leftStickSmoothSize).toInt();
    changeSmoothingSize(0, leftStickSmoothSize);
    changeSmoothingSize(1, leftStickSmoothSize);

    int rightStickSmoothSize = settings->value("rightStickSmoothSize",
                                               ProgramDefaults::rightStickSmoothSize).toInt();
    changeSmoothingSize(2, rightStickSmoothSize);
    changeSmoothingSize(3, rightStickSmoothSize);

    int leftStickSmoothWeight = settings->value("leftStickSmoothWeight",
                                                ProgramDefaults::leftStickSmoothWeight).toInt();
    changeSmoothingWeight(0, leftStickSmoothWeight / 100.0);
    changeSmoothingWeight(1, leftStickSmoothWeight / 100.0);

    double rightStickSmoothWeight = settings->value("rightStickSmoothWeight",
                                                    ProgramDefaults::rightStickSmoothWeight).toInt();
    changeSmoothingWeight(2, rightStickSmoothWeight / 100.0);
    changeSmoothingWeight(3, rightStickSmoothWeight / 100.0);

    QString leftStickCurve = settings->value("leftStickDefaultCurve",
                                             ProgramDefaults::leftStickDefaultCurve).toString();
    AxisCurve::Type leftCurveValue = getCurveFromString(leftStickCurve);
    axisCurves[0] = leftCurveValue;
    axisCurves[1] = leftCurveValue;

    QString rightStickCurve = settings->value("rightStickDefaultCurve",
                                              ProgramDefaults::rightStickDefaultCurve).toString();
    AxisCurve::Type rightCurveValue = getCurveFromString(rightStickCurve);
    axisCurves[2] = rightCurveValue;
    axisCurves[3] = rightCurveValue;

    int leftStickScale = settings->value("leftStickScale",
                                         ProgramDefaults::leftStickScale).toInt();

    changeAxisScale(0, leftStickScale / 100.0);
    changeAxisScale(1, leftStickScale / 100.0);

    int rightStickScale = settings->value("rightStickScale",
                                          ProgramDefaults::rightStickScale).toInt();

    changeAxisScale(2, rightStickScale / 100.0);
    changeAxisScale(3, rightStickScale / 100.0);

    int leftStickDeadZone = settings->value("leftStickDeadZone",
                                            ProgramDefaults::leftStickDeadZone).toInt();
    changeAxisDeadZonePercentage(0, leftStickDeadZone);
    changeAxisDeadZonePercentage(1, leftStickDeadZone);

    int rightStickDeadZone = settings->value("rightStickDeadZone",
                                             ProgramDefaults::rightStickDeadZone).toInt();
    changeAxisDeadZonePercentage(2, rightStickDeadZone);
    changeAxisDeadZonePercentage(3, rightStickDeadZone);

    int leftStickAntiDeadZone = settings->value("leftStickAntiDeadZone",
                                                ProgramDefaults::leftStickAntiDeadZone).toInt();
    changeAxisAntiDeadZonePercentage(0, leftStickAntiDeadZone);
    changeAxisAntiDeadZonePercentage(1, leftStickAntiDeadZone);

    int rightStickAntiDeadZone = settings->value("rightStickAntiDeadZone",
                                                 ProgramDefaults::rightStickAntiDeadZone).toInt();
    changeAxisAntiDeadZonePercentage(2, rightStickAntiDeadZone);
    changeAxisAntiDeadZonePercentage(3, rightStickAntiDeadZone);

    int leftStickMaxZone = settings->value("leftStickMaxZone",
                                           ProgramDefaults::leftStickMaxZone).toInt();
    changeAxisMaxZonePercentage(0, leftStickMaxZone);
    changeAxisMaxZonePercentage(1, leftStickMaxZone);

    int rightStickMaxZone = settings->value("rightStickMaxZone",
                                            ProgramDefaults::rightStickMaxZone).toInt();
    changeAxisMaxZonePercentage(2, rightStickMaxZone);
    changeAxisMaxZonePercentage(3, rightStickMaxZone);

    double leftStickSens = settings->value("leftStickSens",
                                           ProgramDefaults::leftStickSens).toDouble();
    changeAxisSens(0, leftStickSens);
    changeAxisSens(1, leftStickSens);

    double rightStickSens = settings->value("rightStickSens",
                                           ProgramDefaults::rightStickSens).toDouble();
    changeAxisSens(2, rightStickSens);
    changeAxisSens(3, rightStickSens);
}

void InputController::changeAxisCurve(int axis, int curve)
{
    if (axis >= 0 && axis < MAXAXES)
    {
        if (curve == 0)
        {
            axisCurves[axis] = AxisCurve::Linear;
        }
        else if (curve == 1)
        {
            axisCurves[axis] = AxisCurve::EnhancedPrecision;
        }
        else if (curve == 2)
        {
            axisCurves[axis] = AxisCurve::Quadratic;
        }
        else if (curve == 3)
        {
            axisCurves[axis] = AxisCurve::Cubic;
        }
        else if (curve == 4)
        {
            axisCurves[axis] = AxisCurve::Power;
        }
        else if (curve == 5)
        {
            axisCurves[axis] = AxisCurve::Disabled;
        }
    }
}

int InputController::getCurvedAxisValue(AxisCurve::Type curve, int value, int axis)
{
    Q_UNUSED(axis);

    AxisCurve::Type tempCurve = curve;
    int result = value;

    switch (tempCurve)
    {
        case AxisCurve::Linear:
        {
            result = result;
            break;
        }
        case AxisCurve::EnhancedPrecision:
        {
            int sign = result > 0 ? 1 :-1;
            // Normalize axis value with asymmetric max points
            double temp = qAbs(result / ((sign > 0) ? 32767.0 : -32768.0));
            double outputTemp = temp;
            if (temp <= 0.4)
            {
                outputTemp = temp * 0.38;
            }
            else if (temp <= 0.75)
            {
                outputTemp = temp - 0.248;
            }
            else if (temp > 0.75)
            {
                outputTemp = (temp * 1.992) - 0.992;
            }

            // Take original sign into account and compute output value
            result = outputTemp * (sign > 0 ? 32767 : -32768);
            break;
        }
        case AxisCurve::Quadratic:
        {
            int sign = result > 0 ? 1 :-1;
            int temp = result;
            // Normalize axis value with asymmetric max points
            double normalize = qAbs(result / ((sign > 0) ? 32767.0 : -32768.0));
            double outputTemp = temp;
            outputTemp = (normalize * normalize);
            // Take original sign into account and compute output value
            result = outputTemp * (sign > 0 ? 32767 : -32768);

            break;
        }
        case AxisCurve::Cubic:
        {
            int sign = result > 0 ? 1 :-1;
            int temp = result;
            Q_UNUSED(sign);
            // Normalize axis value with asymmetric max points
            double normalize = qAbs(result / ((sign > 0) ? 32767.0 : -32768.0));
            double outputTemp = temp;

            outputTemp = normalize * normalize * normalize;

            // Take original sign into account and compute output value
            result = qAbs(outputTemp) * (sign > 0 ? 32767 : -32768);
            break;
        }
        case AxisCurve::Power:
        {
            int sign = result >= 0 ? 1 :-1;
            double difference = qAbs(result / ((result >= 0) ? static_cast<double>(AXIS_MAX) :
                                                               static_cast<double>(AXIS_MIN)));
            double sensitivity = axisSens[axis];
            double tempsensitive = qMin(qMax(sensitivity, 1.0e-3), 1.0e+3);
            double temp = qMin(qMax(pow(difference, 1.0 / tempsensitive), 0.0), 1.0);
            result = temp * (sign > 0 ? AXIS_MAX : AXIS_MIN);
            break;
        }
        case AxisCurve::Disabled:
        {
            // Always return axis as centered
            result = 0;
            break;
        }
    }

    return result;
}


AxisCurve::Type InputController::getCurveFromString(QString value)
{
    AxisCurve::Type result = AxisCurve::Linear;
    if (value == "linear")
    {
        result = AxisCurve::Linear;
    }
    else if (value == "enhanced-precision")
    {
        result = AxisCurve::EnhancedPrecision;
    }
    else if (value == "quadratic")
    {
        result = AxisCurve::Quadratic;
    }
    else if (value == "cubic")
    {
        result = AxisCurve::Cubic;
    }
    else if (value == "power")
    {
        result = AxisCurve::Power;
    }
    else if (value == "disabled")
    {
        result = AxisCurve::Disabled;
    }

    return result;
}

void InputController::changeAxisScale(int axis, double value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0.0 && value <= 2.0)
    {
        if (value != axisScales[axis])
        {
            axisScales[axis] = value;
        }
    }
}

void InputController::changeAxisDeadZonePercentage(int axis, int value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100)
    {
        axisDeadZones[axis] = value;
    }
}

void InputController::changeAxisAntiDeadZonePercentage(int axis, int value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100)
    {
        axisAntiDeadZones[axis] = value;
    }
}

void InputController::changeAxisMaxZonePercentage(int axis, int value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100)
    {
        axisMaxZones[axis] = value;
    }
}

void InputController::changeAxisSens(int axis, double value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100.0)
    {
        axisSens[axis] = value;
    }
}

int InputController::calculateAxisValueAfterDead(int axis, int value)
{
    int result = value;
    int axisDeadZone = axisDeadZones[axis];
    int axisAntiDeadZone = axisAntiDeadZones[axis];

    // If no calculation needs to be done, save time and skip.
    if (axisDeadZone != 0 || axisAntiDeadZone != 0)
    {
        int maxDirValue = (value > 0) ? AXIS_MAX : AXIS_MIN;
        int currentDead = (axisDeadZone / 100.0) * maxDirValue;
        double currentAntiDead = (axisAntiDeadZone / 100.0);
        // Obtain normalized magnitude
        double tempValue = (qAbs(value) >= qAbs(currentDead)) ?
                    (value - currentDead) / static_cast<double>(maxDirValue - currentDead) :
                    0.0;

        if (tempValue > 0.0)
        {
            // Calculate anti-dead zone offset
            tempValue = (1.0 - currentAntiDead) * tempValue + currentAntiDead;
        }

        // Convert normalized value back to full range
        result = qFloor(tempValue * maxDirValue);
    }

    return result;
}

void InputController::calculateStickValuesAfterDead(int axis1, int axis2, int axis1Value,
                                                    int axis2Value, int &outAxis1Value,
                                                    int &outAxis2Value)
{
    int axisXDeadTemp = axisDeadZones[axis1];
    int axisYDeadTemp = axisDeadZones[axis2];
    int axisXAntiDeadTemp = axisAntiDeadZones[axis1];
    int axisYAntiDeadTemp = axisAntiDeadZones[axis2];
    int axisXMaxTemp = axisMaxZones[axis1];
    int axisYMaxTemp = axisMaxZones[axis2];

    // If no calculation needs to be done, save time and skip.
    if (axisXDeadTemp != 0 || axisYDeadTemp != 0 ||
        axisXAntiDeadTemp != 0 || axisYAntiDeadTemp != 0 ||
        axisXMaxTemp != 100 || axisYMaxTemp != 100)
    {
        double angle = atan2(axis1Value, -axis2Value);
        double ang_sin = sin(angle);
        double ang_cos = cos(angle);

        int maxXDirValue = (axis1Value >= 0) ? AXIS_MAX : AXIS_MIN;
        int maxYDirValue = (axis2Value >= 0) ? AXIS_MAX : AXIS_MIN;

        //int maxzoneValue = 30000;
        //double maxzoneDoubValue = 0.92;
        //int maxzoneXDirValue = maxzoneValue * (axis1Value >= 0 ? 1 : -1);
        //int maxzoneYDirValue = maxzoneValue * (axis2Value >= 0 ? 1 : -1);

        int maxzoneXNegDirValue = (axisXMaxTemp / 100.0) * AXIS_MIN;
        int maxzoneXPosDirValue = (axisXMaxTemp / 100.0) * AXIS_MAX;
        int maxzoneYNegDirValue = (axisYMaxTemp / 100.0) * AXIS_MIN;
        int maxzoneYPosDirValue = (axisYMaxTemp / 100.0) * AXIS_MAX;

        int maxzoneXDirValue = (axis1Value >= 0 ? maxzoneXPosDirValue : maxzoneXNegDirValue);
        int maxzoneYDirValue = (axis2Value >= 0 ? maxzoneYPosDirValue : maxzoneYNegDirValue);

        int oldAxis1Value = axis1Value;

        //axis1Value = qBound(-maxzoneValue, axis1Value, maxzoneValue);
        //axis2Value = qBound(-maxzoneValue, axis2Value, maxzoneValue);

        axis1Value = qBound(maxzoneXNegDirValue, axis1Value, maxzoneXPosDirValue);
        axis2Value = qBound(maxzoneYNegDirValue, axis2Value, maxzoneYPosDirValue);

        int currentDeadX = qFloor((axisXDeadTemp / 100.0 * maxXDirValue) * qAbs(ang_sin));
        int currentDeadY = qFloor((axisYDeadTemp / 100.0 * maxYDirValue) * qAbs(ang_cos));

        double currentXAntiDead = (axisXAntiDeadTemp / 100.0) * qAbs(ang_sin);
        double currentYAntiDead = (axisYAntiDeadTemp / 100.0) * qAbs(ang_cos);

        double tempXValue = 0.0;
        double tempYValue = 0.0;

        //double tempXValueBefore = 0.0;
        //double tempYValueBefore = 0.0;

        int squareDist = (axis1Value * axis1Value) + (axis2Value * axis2Value);
        int deadDist = (currentDeadX * currentDeadX) + (currentDeadY * currentDeadY);
        if (squareDist > deadDist)
        {
            // Obtain normalized magnitude
            tempXValue = (axis1Value - currentDeadX) /
                    static_cast<double>(maxzoneXDirValue - currentDeadX);

            tempYValue = (axis2Value - currentDeadY) /
                    static_cast<double>(maxzoneYDirValue - currentDeadY);

            //tempXValueBefore = tempXValue;
            //tempYValueBefore = tempYValue;
        }

        if (tempXValue > 0.0)
        {
            // Calculate anti-dead zone offset
            tempXValue = (1.0 - currentXAntiDead) * tempXValue + currentXAntiDead;
        }

        if (tempYValue > 0.0)
        {
            // Calculate anti-dead zone offset
            tempYValue = (1.0 - currentYAntiDead) * tempYValue + currentYAntiDead;
        }

        // Convert normalized value back to full range
        outAxis1Value = qFloor(tempXValue * maxXDirValue);
        outAxis2Value = qFloor(tempYValue * maxYDirValue);

        if (axis1 == 2)
        {
            //qDebug() << "VALUES: " << oldAxis1Value << " | " << outAxis1Value << " | " << maxzoneXDirValue;
        }
    }
}

int InputController::getAxisMaxZoneNegValue(int axis)
{
    int result;

    if (axis >= 0 && axis < MAXAXES)
    {
        int axisMaxTemp = axisMaxZones[axis];

        int maxzoneNegDirValue = (axisMaxTemp / 100.0) * AXIS_MIN;
        result = maxzoneNegDirValue;
    }

    return result;
}

int InputController::getAxisMaxZonePosValue(int axis)
{
    int result;

    if (axis >= 0 && axis < MAXAXES)
    {
        int axisMaxTemp = axisMaxZones[axis];

        int maxzonePosDirValue = (axisMaxTemp / 100.0) * AXIS_MAX;
        result = maxzonePosDirValue;
    }

    return result;
}

int InputController::calculateFlippedAxisValue(int value)
{
    int result = value;
    int maxNegDirValue = AXIS_MIN;
    int maxPosDirValue = AXIS_MAX;
    int maxDirValue = (value >= 0 ? (value / static_cast<double>(maxPosDirValue)) * maxNegDirValue : (value / static_cast<double>(maxNegDirValue)) * maxPosDirValue);
    result = maxDirValue;
    return result;
}
