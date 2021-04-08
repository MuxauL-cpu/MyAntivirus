#ifndef BASEEDITOR_H
#define BASEEDITOR_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QClipboard>
#include <QToolTip>
#include <QTime>
#include <QTimer>
#include <windows.h>
#include "IPC.h"
#include <SHA256.h>

QT_BEGIN_NAMESPACE
namespace DataBaseUI { class BaseEditor; }
QT_END_NAMESPACE

class BaseEditor : public QMainWindow
{
    Q_OBJECT

public:
    BaseEditor(QWidget *parent = nullptr);
    ~BaseEditor();

private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_saveButton_clicked();

    void on_loadButton_clicked();

    void on_baseTable_itemChanged(QTableWidgetItem *item);

    void on_baseTable_itemClicked(QTableWidgetItem *item);

    void on_baseTable_itemDoubleClicked(QTableWidgetItem *item);

    void update();

private:
    DataBaseUI::BaseEditor *ui;
    QTimer* timer;
    qint64 m_tooltipTimerStart;
    HANDLE hFile;
    void Save();
    void Load();
    void setHash();
};
#endif // BASEEDITOR_H
