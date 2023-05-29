#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QTreeView>
#include <QByteArray>
#include <QMessageBox>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QTimer>
#include <QMessageBox>
#include "workthread.h"
#include "appvars.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void UpdateProgress(qint64 Position, qint64 Size);
    void FileComplete(int index, int count);
    void OperationComplete();
    void CheckUpdates();
    void on_btnGoFile_clicked();
    void on_checkEncode_toggled(bool checked);
    void on_checkDecode_toggled(bool checked);
    void on_btnAddFile_clicked();
    void on_btnDeleteFile_clicked();
    void on_btnClearList_clicked();
    void on_btnPickDir_clicked();
    void on_checkEncodeText_toggled(bool checked);
    void on_checkDecodeText_toggled(bool checked);
    void on_btnGoText_clicked();
    void on_btnClearText_clicked();
    void on_btnAbout_clicked();
    void on_checkImgTag_clicked();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

private:
    Ui::MainWindow *ui;
    int CurrentIndex;
    int FileCount;
    WorkThread *worker;
    QLabel *labelUpdate;
    void SetControlsActive(bool m);

};

#endif // MAINWINDOW_H
