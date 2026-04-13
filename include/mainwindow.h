#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDateTime>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QRect>

#include "tokenringdrawer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void addToLog(const QString& message);
    ~MainWindow();
public slots:
    void startSimulation();
    void stopSimulation();
    void updateStationsCount(const QString& text);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    void updateComboBoxes(int stationCount);
    Ui::MainWindow *ui;
    TokenRingDrawer* tokenRingDrawer = nullptr;
    QGraphicsScene* tokenRingScene;
    QGraphicsView* tokenRingSceneView;
    QTextEdit* logsTextEdit;
    int stationsCount = 8;
    QComboBox* senderComboBox;
    QComboBox* receiverComboBox;
    QLineEdit* stationsCountInput;
    QDialog* errorDialog;
    QDialog* stationDialog;
    QDialog* markerDialog;

};
#endif // MAINWINDOW_H
