#ifndef TESTMAINWINDOWNOW_H
#define TESTMAINWINDOWNOW_H

#include <QMainWindow>
#include <QSettings>

#include "inputcontroller.h"
#include "sdleventqueue.h"

namespace Ui {
class TestMainWindowNOw;
}

class TestMainWindowNOw : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestMainWindowNOw(SDLEventQueue *eventQueue,
                               QSettings *settings, QWidget *parent = 0);
    ~TestMainWindowNOw();

protected:
    void readSettings(QSettings *settings);
    int getCurveComboIndex(QString value);
    QString getCurveCode(int index);

    SDLEventQueue *eventQueue;
    QSettings *settings;

protected slots:
    void changeStateLWidgets(int value);
    void changeStateRWidgets(int value);
    void changeSmoothingLStatus(int value);
    void changeSmoothingRStatus(int value);
    void changePollRate(const QString &value);
    void changePollRateChange(int index);
    void changeSmoothingLSize();
    void changeSmoothingRSize();
    void changeSmoothingLWeight();
    void changeSmoothingRWeight();
    void changeAxisCurveL(int index);
    void changeAxisCurveR(int index);
    void changeLStickScale();
    void changeRStickScale();
    void changeLStickDeadZone();
    void changeRStickDeadZone();
    void changeLStickAntiDeadZone();
    void changeRStickAntiDeadZone();

private:
    Ui::TestMainWindowNOw *ui;
};

#endif // TESTMAINWINDOWNOW_H
