#include <QtMath>

#include "testmainwindownow.h"
#include "ui_testmainwindownow.h"
#include "defaultsettings.h"

TestMainWindowNOw::TestMainWindowNOw(SDLEventQueue *eventQueue,
                                     QSettings *settings, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestMainWindowNOw)
{
    ui->setupUi(this);
    this->eventQueue = eventQueue;
    this->settings = settings;

    for (int i=1; i <= 16; i++)
    {
        ui->pollrateComboBox->addItem(QString::number(i), i);
    }

    readSettings(settings);

    connect(ui->smoothingLCheckBox, &QCheckBox::stateChanged, this, &TestMainWindowNOw::changeStateLWidgets);
    connect(ui->smoothingRCheckBox, &QCheckBox::stateChanged, this, &TestMainWindowNOw::changeStateRWidgets);

    connect(ui->smoothingLCheckBox, &QCheckBox::stateChanged, this, &TestMainWindowNOw::changeSmoothingLStatus);
    connect(ui->smoothingRCheckBox, &QCheckBox::stateChanged, this, &TestMainWindowNOw::changeSmoothingRStatus);

    connect(ui->pollrateComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changePollRate(QString)));
    connect(ui->pollrateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changePollRateChange(int)));
    //connect(ui->pollrateComboBox, &QComboBox::currentIndexChanged, this, &TestMainWindowNOw::changePollRate);
    //connect(ui->pollrateComboBox, &QComboBox::currentIndexChanged, this, &TestMainWindowNOw::changePollRateChange);

    connect(ui->smoothingSizeLSpinBox, SIGNAL(editingFinished()), this, SLOT(changeSmoothingLSize()));
    connect(ui->smoothingSizeRSpinBox, SIGNAL(editingFinished()), this, SLOT(changeSmoothingRSize()));

    connect(ui->smoothingWeightLSpinBox, SIGNAL(editingFinished()), this, SLOT(changeSmoothingLWeight()));
    connect(ui->smoothingWeightRSpinBox, SIGNAL(editingFinished()), this, SLOT(changeSmoothingRWeight()));

    connect(ui->leftStickAxisCurveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeAxisCurveL(int)));
    connect(ui->rightStickAxisCurveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeAxisCurveR(int)));

    connect(ui->leftStickScaleSpinBox, SIGNAL(editingFinished()), this, SLOT(changeLStickScale()));
    connect(ui->rightStickScaleSpinBox, SIGNAL(editingFinished()), this, SLOT(changeRStickScale()));

    connect(ui->leftStickDeadZoneSpinBox, SIGNAL(editingFinished()), this, SLOT(changeLStickDeadZone()));
    connect(ui->rightStickDeadZoneSpinBox, SIGNAL(editingFinished()), this, SLOT(changeRStickDeadZone()));

    connect(ui->leftStickAntiDeadSpinBox, SIGNAL(editingFinished()), this, SLOT(changeLStickAntiDeadZone()));
    connect(ui->rightStickAntiDeadSpinBox, SIGNAL(editingFinished()), this, SLOT(changeRStickAntiDeadZone()));

    connect(ui->leftStickMaxZoneSpinBox, SIGNAL(editingFinished()), this, SLOT(changeLStickMaxZone()));
    connect(ui->rightStickMaxZoneSpinBox, SIGNAL(editingFinished()), this, SLOT(changeRStickMaxZone()));

    connect(ui->leftStickMaxZoneSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLStickMaxZoneSample()));
    connect(ui->rightStickMaxZoneSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRStickMaxZoneSample()));

    connect(ui->leftStickAntiDeadSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLStickAntiDeadSample()));
    connect(ui->rightStickAntiDeadSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRStickAntiDeadSample()));
}

TestMainWindowNOw::~TestMainWindowNOw()
{
    delete ui;
}

void TestMainWindowNOw::changeStateLWidgets(int value)
{
    if (value)
    {
        ui->smoothingSizeLSpinBox->setEnabled(true);
        ui->smoothingWeightLSpinBox->setEnabled(true);
    }
    else
    {
        ui->smoothingSizeLSpinBox->setEnabled(false);
        ui->smoothingWeightLSpinBox->setEnabled(false);
    }
}

void TestMainWindowNOw::changeStateRWidgets(int value)
{
    if (value)
    {
        ui->smoothingSizeRSpinBox->setEnabled(true);
        ui->smoothingWeightRSpinBox->setEnabled(true);
    }
    else
    {
        ui->smoothingSizeRSpinBox->setEnabled(false);
        ui->smoothingWeightRSpinBox->setEnabled(false);
    }
}

void TestMainWindowNOw::changeSmoothingLStatus(int value)
{
    bool enabled = value > 0 ? true : false;
    settings->setValue("leftStickSmoothEnabled", enabled);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingStatus", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(bool, enabled));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingStatus", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(bool, enabled));
    //eventQueue->getController()->changeSmoothingStatus(0, enabled);
    //eventQueue->getController()->changeSmoothingStatus(1, enabled);
}

void TestMainWindowNOw::changeSmoothingRStatus(int value)
{
    bool enabled = value > 0 ? true : false;
    settings->setValue("rightStickSmoothEnabled", enabled);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingStatus", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(bool, enabled));

    //eventQueue->getController()->changeSmoothingStatus(3, enabled);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingStatus", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(bool, enabled));
}


void TestMainWindowNOw::changePollRate(const QString &value)
{
    int temp = value.toInt();
    //eventQueue->changePollRate(temp);
    settings->setValue("pollRate", temp);
    QMetaObject::invokeMethod(eventQueue, "changePollRate", Qt::AutoConnection, Q_ARG(int, temp));
}

void TestMainWindowNOw::changePollRateChange(int index)
{
    int temp = ui->pollrateComboBox->itemData(index).toInt();
    //eventQueue->changePollRate(temp);
    settings->setValue("pollRate", temp);
    QMetaObject::invokeMethod(eventQueue, "changePollRate", Qt::AutoConnection, Q_ARG(int, temp));
}

void TestMainWindowNOw::changeSmoothingLSize()
{
    int value = ui->smoothingSizeLSpinBox->value();

    settings->setValue("leftStickSmoothSize", value);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingSize", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingSize", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(int, value));
}

void TestMainWindowNOw::changeSmoothingRSize()
{
    int value = ui->smoothingSizeRSpinBox->value();

    settings->setValue("rightStickSmoothSize", value);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingSize", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingSize", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(int, value));
}

void TestMainWindowNOw::changeSmoothingLWeight()
{
    int value = ui->smoothingWeightLSpinBox->value();
    double temp = value / 100.0;

    settings->setValue("leftStickSmoothWeight", value);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingWeight", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(double, temp));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingWeight", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(double, temp));
}

void TestMainWindowNOw::changeSmoothingRWeight()
{
    int value = ui->smoothingWeightRSpinBox->value();
    double temp = value / 100.0;

    settings->setValue("rightStickSmoothWeight", value);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingWeight", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(double, temp));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeSmoothingWeight", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(double, temp));
}

void TestMainWindowNOw::readSettings(QSettings *settings)
{
    int index = ui->pollrateComboBox->findData(
                settings->value("pollRate", ProgramDefaults::pollRate).toInt());
    if (index >= 0)
    {
        ui->pollrateComboBox->setCurrentIndex(index);
    }

    bool leftStickSmooth = settings->value("leftStickSmoothEnabled",
                                           ProgramDefaults::leftStickSmoothEnabled).toBool();
    if (leftStickSmooth)
    {
        ui->smoothingLCheckBox->setChecked(true);
        ui->smoothingSizeLSpinBox->setEnabled(true);
        ui->smoothingWeightLSpinBox->setEnabled(true);
    }

    bool rightStickSmooth = settings->value("rightStickSmoothEnabled",
                                            ProgramDefaults::rightStickSmoothEnabled).toBool();
    if (rightStickSmooth)
    {
        ui->smoothingRCheckBox->setChecked(true);
        ui->smoothingSizeRSpinBox->setEnabled(true);
        ui->smoothingWeightRSpinBox->setEnabled(true);
    }

    int leftStickSmoothSize = settings->value("leftStickSmoothSize",
                                              ProgramDefaults::leftStickSmoothSize).toInt();
    ui->smoothingSizeLSpinBox->setValue(leftStickSmoothSize);

    int rightStickSmoothSize = settings->value("rightStickSmoothSize",
                                               ProgramDefaults::rightStickSmoothSize).toInt();
    ui->smoothingSizeRSpinBox->setValue(rightStickSmoothSize);

    int leftStickSmoothWeight = settings->value("leftStickSmoothWeight",
                                                   ProgramDefaults::leftStickSmoothWeight).toInt();
    ui->smoothingWeightLSpinBox->setValue(leftStickSmoothWeight);

    int rightStickSmoothWeight = settings->value("rightStickSmoothWeight",
                                                   ProgramDefaults::rightStickSmoothWeight).toInt();
    ui->smoothingWeightRSpinBox->setValue(rightStickSmoothWeight);

    QString leftStickCurve = settings->value("leftStickDefaultCurve",
                                             ProgramDefaults::leftStickDefaultCurve).toString();
    ui->leftStickAxisCurveComboBox->setCurrentIndex(getCurveComboIndex(leftStickCurve));

    QString rightStickCurve = settings->value("rightStickDefaultCurve",
                                             ProgramDefaults::rightStickDefaultCurve).toString();
    ui->rightStickAxisCurveComboBox->setCurrentIndex(getCurveComboIndex(rightStickCurve));

    int leftStickScale = settings->value("leftStickScale", ProgramDefaults::leftStickScale).toInt();
    ui->leftStickScaleSpinBox->setValue(leftStickScale);

    int rightStickScale = settings->value("rightStickScale", ProgramDefaults::rightStickScale).toInt();
    ui->rightStickScaleSpinBox->setValue(rightStickScale);

    int leftStickDeadZone = settings->value("leftStickDeadZone",
                                            ProgramDefaults::leftStickDeadZone).toInt();
    ui->leftStickDeadZoneSpinBox->setValue(leftStickDeadZone);

    int rightStickDeadZone = settings->value("rightStickDeadZone",
                                            ProgramDefaults::rightStickDeadZone).toInt();
    ui->rightStickDeadZoneSpinBox->setValue(rightStickDeadZone);

    int leftStickAntiDeadZone = settings->value("leftStickAntiDeadZone",
                                                ProgramDefaults::leftStickAntiDeadZone).toInt();
    ui->leftStickAntiDeadSpinBox->setValue(leftStickAntiDeadZone);
    updateLStickAntiDeadSample();

    int rightStickAntiDeadZone = settings->value("rightStickAntiDeadZone",
                                                ProgramDefaults::rightStickAntiDeadZone).toInt();
    ui->rightStickAntiDeadSpinBox->setValue(rightStickAntiDeadZone);
    updateRStickAntiDeadSample();

    int leftStickMaxZone = settings->value("leftStickMaxZone",
                                           ProgramDefaults::leftStickMaxZone).toInt();
    ui->leftStickMaxZoneSpinBox->setValue(leftStickMaxZone);
    //QTimer::singleShot(0, this, updateLStickMaxZoneSample());
    updateLStickMaxZoneSample();

    int rightStickMaxZone = settings->value("rightStickMaxZone",
                                            ProgramDefaults::rightStickMaxZone).toInt();
    ui->rightStickMaxZoneSpinBox->setValue(rightStickMaxZone);
    //QTimer::singleShot(0, this, updateRStickMaxZoneSample());
    updateRStickMaxZoneSample();
}

void TestMainWindowNOw::changeAxisCurveL(int index)
{
    QString configValue = getCurveCode(index);
    settings->setValue("leftStickDefaultCurve", configValue);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisCurve", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(int, index));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisCurve", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(int, index));
}

void TestMainWindowNOw::changeAxisCurveR(int index)
{

    QString configValue = getCurveCode(index);
    settings->setValue("rightStickDefaultCurve", configValue);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisCurve", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(int, index));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisCurve", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(int, index));
}

int TestMainWindowNOw::getCurveComboIndex(QString value)
{
    int index = 0;
    if (value == "linear")
    {
        index = 0;
    }
    else if (value == "enhanced-precision")
    {
        index = 1;
    }
    else if (value == "quadratic")
    {
        index = 2;
    }
    else if (value == "cubic")
    {
        index = 3;
    }
    else if (value == "disabled")
    {
        index = 4;
    }

    return index;
}

QString TestMainWindowNOw::getCurveCode(int index)
{
    QString result = "linear";
    if (index == 0)
    {
        result = "linear";
    }
    else if (index == 1)
    {
        result = "enhanced-precision";
    }
    else if (index == 2)
    {
        result = "quadratic";
    }
    else if (index == 3)
    {
        result = "cubic";
    }
    else if (index == 4)
    {
        result = "disabled";
    }

    return result;
}

void TestMainWindowNOw::changeLStickScale()
{
    int value = ui->leftStickScaleSpinBox->value();
    double temp = value / 100.0;

    settings->setValue("leftStickScale", value);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisScale", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(double, temp));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisScale", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(double, temp));
}

void TestMainWindowNOw::changeRStickScale()
{
    int value = ui->rightStickScaleSpinBox->value();
    double temp = value / 100.0;

    settings->setValue("rightStickScale", value);
    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisScale", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(double, temp));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisScale", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(double, temp));
}

void TestMainWindowNOw::changeLStickDeadZone()
{
    int value = ui->leftStickDeadZoneSpinBox->value();
    settings->setValue("leftStickDeadZone", value);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(int, value));
}

void TestMainWindowNOw::changeRStickDeadZone()
{
    int value = ui->rightStickDeadZoneSpinBox->value();
    settings->setValue("rightStickDeadZone", value);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(int, value));
}

void TestMainWindowNOw::changeLStickAntiDeadZone()
{
    int value = ui->leftStickAntiDeadSpinBox->value();
    settings->setValue("leftStickAntiDeadZone", value);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisAntiDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisAntiDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(int, value));
}

void TestMainWindowNOw::changeRStickAntiDeadZone()
{
    int value = ui->rightStickAntiDeadSpinBox->value();
    settings->setValue("rightStickAntiDeadZone", value);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisAntiDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisAntiDeadZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(int, value));
}

void TestMainWindowNOw::changeLStickMaxZone()
{
    int value = ui->leftStickMaxZoneSpinBox->value();
    settings->setValue("leftStickMaxZone", value);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisMaxZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 0), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisMaxZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 1), Q_ARG(int, value));
}

void TestMainWindowNOw::changeRStickMaxZone()
{
    int value = ui->rightStickMaxZoneSpinBox->value();
    settings->setValue("rightStickMaxZone", value);

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisMaxZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 2), Q_ARG(int, value));

    QMetaObject::invokeMethod(eventQueue->getController(),
                              "changeAxisMaxZonePercentage", Qt::AutoConnection,
                              Q_ARG(int, 3), Q_ARG(int, value));
}


void TestMainWindowNOw::updateLStickMaxZoneSample()
{
    QString temp = "(%0,%1)";
    int maxzoneNegDirValue = (ui->leftStickMaxZoneSpinBox->value() / 100.0) *
            InputController::AXIS_MIN;
    int maxzonePosDirValue = (ui->leftStickMaxZoneSpinBox->value() / 100.0) *
            InputController::AXIS_MAX;

    temp = temp.arg(maxzoneNegDirValue).arg(maxzonePosDirValue);
    ui->leftStickMaxZoneSampleLabel->setText(temp);

}

void TestMainWindowNOw::updateRStickMaxZoneSample()
{
    QString temp = "(%0,%1)";
    int maxzoneNegDirValue = (ui->rightStickMaxZoneSpinBox->value() / 100.0) *
            InputController::AXIS_MIN;
    int maxzonePosDirValue = (ui->rightStickMaxZoneSpinBox->value() / 100.0) *
            InputController::AXIS_MAX;

    temp = temp.arg(maxzoneNegDirValue).arg(maxzonePosDirValue);
    ui->rightStickMaxZoneSampleLabel->setText(temp);
}

void TestMainWindowNOw::updateLStickAntiDeadSample()
{
    QString temp = "(%0,%1)";
    int maxzoneNegDirValue = (ui->leftStickAntiDeadSpinBox->value() / 100.0) *
            InputController::AXIS_MIN;
    int maxzonePosDirValue = (ui->leftStickAntiDeadSpinBox->value() / 100.0) *
            InputController::AXIS_MAX;

    temp = temp.arg(maxzoneNegDirValue).arg(maxzonePosDirValue);
    ui->leftStickAntiDeadSampleLabel->setText(temp);
}

void TestMainWindowNOw::updateRStickAntiDeadSample()
{
    QString temp = "(%0,%1)";
    int maxzoneNegDirValue = (ui->rightStickAntiDeadSpinBox->value() / 100.0) *
            InputController::AXIS_MIN;
    int maxzonePosDirValue = (ui->rightStickAntiDeadSpinBox->value() / 100.0) *
            InputController::AXIS_MAX;

    temp = temp.arg(maxzoneNegDirValue).arg(maxzonePosDirValue);
    ui->rightStickAntiDeadSampleLabel->setText(temp);
}
