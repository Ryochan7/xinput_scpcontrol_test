#include <QDebug>

#include "inputcontroller.h"

InputController::InputController(ScpBusDevice *busDevice, QObject *parent) :
    QObject(parent)
{
    this->busDevice = busDevice;
    xinputIndex = 1;
    memset(&buttons, 0, sizeof(buttons));
    memset(&axes, 0, sizeof(axes));
    hat = 0;
}

InputController::InputController(QObject *parent) : QObject(parent)
{
    this->busDevice = 0;
    xinputIndex = 1;
    memset(&buttons, 0, sizeof(buttons));
    memset(&axes, 0, sizeof(axes));
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
        /*qDebug() << "BUTTON STUFF: index(" << buttonIndex << ")";
        if (value)
        {
            qDebug() << "TRUE";
        }
        else
        {
            qDebug() << "FALSE";
        }
        */

        buttons[buttonIndex] = value;
    }
}

void InputController::axisEvent(int axisIndex, int value)
{
    if (axisIndex >= 0 && axisIndex < static_cast<int>(MAXAXES) &&
        value >= -32768 && value <= 32767)
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
    buttonsTest |= (hat & 0x01) > 0 ? DPAD_UP : 0;
    buttonsTest |= (hat & 0x02) > 0 ? DPAD_RIGHT : 0;
    buttonsTest |= (hat & 0x04) > 0 ? DPAD_DOWN : 0;
    buttonsTest |= (hat & 0x08) > 0 ? DPAD_LEFT : 0;

    buttonsTest |= buttons[STARTPS1] ? START : 0;
    buttonsTest |= buttons[SELECT] ? BACK : 0;
    buttonsTest |= buttons[L3] ? LEFTSTICK : 0;
    buttonsTest |= buttons[R3] ? RIGHTSTICK : 0;
    buttonsTest |= buttons[L1] ? LEFTBUMPER : 0;
    buttonsTest |= buttons[R1] ? RIGHTBUMPER : 0;

    buttonsTest |= buttons[CROSS] ? A : 0;
    buttonsTest |= buttons[CIRCLE] ? B : 0;
    buttonsTest |= buttons[SQUARE] ? X : 0;
    buttonsTest |= buttons[TRIANGLE] ? Y : 0;

    report[2] = (byte)(buttonsTest >> 0 & 0xFF);  // Buttons low
    report[3] = (byte)(buttonsTest >> 8 & 0xFF);  // Buttons high

    report[4] = buttons[L2] ? 255 : 0;           // Left trigger
    report[5] = buttons[R2] ? 255 : 0;           // Right trigger

    short lxAxis = static_cast<short>(qBound(-32767, axes[0], 32767));
    report[6] = static_cast<byte>(lxAxis & 0xFF);           // Left stick X-axis low
    report[7] = static_cast<byte>(lxAxis >> 8 & 0xFF);      // Left stick X-axis high

    short lyAxis = static_cast<short>(qBound(-32767, -axes[1], 32767));
    report[8] = static_cast<byte>(lyAxis & 0xFF);           // Left stick Y-axis low
    report[9] = static_cast<byte>(lyAxis >> 8 & 0xFF);      // Left stick Y-axis high

    //qDebug() << "AXIS 1: " << axes[1];
    //qDebug() << "AXIS 2: " << axes[2];

    short rxAxis = static_cast<short>(qBound(-32767, axes[3], 32767));
    report[10] = static_cast<byte>(rxAxis & 0xFF);          // Right stick X-axis low
    report[11] = static_cast<byte>(rxAxis >> 8 & 0xFF);     // Right stick X-axis high

    short ryAxis = static_cast<short>(qBound(-32767, -axes[2], 32767));
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
