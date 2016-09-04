#ifndef TESTMAINWINDOWNOW_H
#define TESTMAINWINDOWNOW_H

#include <QMainWindow>

namespace Ui {
class TestMainWindowNOw;
}

class TestMainWindowNOw : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestMainWindowNOw(QWidget *parent = 0);
    ~TestMainWindowNOw();

private:
    Ui::TestMainWindowNOw *ui;
};

#endif // TESTMAINWINDOWNOW_H
