#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
QString GetWinVer() {
    // http://web3.codeproject.com/Messages/4823136/Re-Another-Way.aspx
    NTSTATUS (WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;

    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");
    QString result;
    if (NULL != RtlGetVersion) {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        result = "Windows ";
        if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) result += "NT ";
        result += QString::number(osInfo.dwMajorVersion) + "." + QString::number(osInfo.dwMinorVersion);
    }
    return result;
}
#endif

QString GetOS() {
    QString OS;
#ifdef Q_OS_LINUX
    OS = "X11; Linux";
#endif
#ifdef Q_OS_FREEBSD
    OS = "X11; FreeBSD";
#endif
#ifdef Q_OS_WIN
    OS = GetWinVer();
#endif
#ifdef Q_OS_MAC
    OS = "Macintosh; Mac OSX";
#endif
    return OS;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowTitle(APPNAME);
    this->setAcceptDrops(true);
    ui->checkEncode->setChecked(true);
    ui->btnGoFile->setText("Encode");
    ui->checkEncodeText->setChecked(true);
    ui->btnGoText->setText("Encode");
    QTimer::singleShot(200, this, SLOT(CheckUpdates()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::UpdateProgress(qint64 Position, qint64 Size) {
    QString pos = QString::number(Position);
    QString size = QString::number(Size);
    QString msg = "["+ QString::number(CurrentIndex) + "/" + QString::number(FileCount) + "] Processing " + pos + " of " + size + " bytes";
    ui->statusBar->showMessage(msg);
}

void MainWindow::FileComplete(int index, int count) {
    CurrentIndex = index;
    FileCount = count;
}

void MainWindow::OperationComplete() {
    ui->statusBar->showMessage("Done");
    SetControlsActive(true);
    if(ui->btnGoFile->text() == "Stop") {
        ui->btnGoFile->setEnabled(true);
        if(ui->checkEncode->isChecked()) {
            ui->btnGoFile->setText("Encode");
        }
        else {
            ui->btnGoFile->setText("Decode");
        }
    }
}

void MainWindow::on_btnGoFile_clicked(){
    if(ui->btnGoFile->text() == "Stop") {
        worker->DoStop();
        ui->btnGoFile->setEnabled(false);
        return;
    }
    if(ui->textOutputDirectory->text().isEmpty() && !ui->checkImgTag->isChecked()) {
        QMessageBox ebox;
        ebox.setIcon(QMessageBox::Critical);
        ebox.setWindowTitle("Error");
        ebox.setText("Output directory not specified!");
        ebox.exec();
        return;
    }

    QFileInfo fo(ui->textOutputDirectory->text());
    if(!fo.exists() && !ui->checkImgTag->isChecked()) {
        QMessageBox ebox;
        ebox.setIcon(QMessageBox::Critical);
        ebox.setWindowTitle("Error");
        ebox.setText("Output directory does not exist!");
        ebox.exec();
        return;
    }
    SetControlsActive(false);
    ui->btnGoFile->setText("Stop");
    QThread* thread = new QThread;
    //WorkThread* worker = new WorkThread();
    worker = new WorkThread();
    CurrentIndex = 1;
    FileCount = 0;
    int i;
    for(i = 0; i < ui->listFiles->count(); i++) {
        QListWidgetItem *item = ui->listFiles->item(i);
        QFileInfo fi(item->text());
        if(fi.exists()) {
            worker->AddFile(item->text());
            FileCount++;
        }
    }
    if(ui->checkImgTag->isChecked()) {
        worker->SetMode(wmImage);
    }
    else {
        if(ui->checkEncode->isChecked()) {
            worker->SetMode(wmEncode);
        }
        else {
            worker->SetMode(wmDecode);
        }
    }
    worker->SetOutputPath(ui->textOutputDirectory->text());
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, SIGNAL(Finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(Finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(worker, SIGNAL(UpdateProgress(qint64,qint64)), this, SLOT(UpdateProgress(qint64,qint64)));
    connect(worker, SIGNAL(FileComplete(int,int)), this, SLOT(FileComplete(int,int)));
    connect(worker, SIGNAL(Finished()), this, SLOT(OperationComplete()));    
    thread->start();
}

void MainWindow::on_checkEncode_toggled(bool checked) {
    if(checked) {
        ui->btnGoFile->setText("Encode");
    }
}

void MainWindow::on_checkDecode_toggled(bool checked) {
    if(checked) {
        ui->btnGoFile->setText("Decode");
    }
}

void MainWindow::on_btnAddFile_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Select files", "", "*.*");
    if(filenames.isEmpty()) return;
    ui->listFiles->addItems(filenames);
}

void MainWindow::on_btnDeleteFile_clicked() {
    qDeleteAll(ui->listFiles->selectedItems());
}

void MainWindow::on_btnClearList_clicked() {
    ui->listFiles->clear();
}

void MainWindow::on_btnPickDir_clicked() {
    if(!ui->checkImgTag->isChecked()) {
        QString outputdir = QFileDialog::getExistingDirectory(this, "Select directory");
        if(outputdir.isEmpty()) return;
        ui->textOutputDirectory->setText(outputdir);
    }
    else {
        QString outputfile = QFileDialog::getSaveFileName(this, "Choose file","","*.html");
        if(outputfile.isEmpty()) return;
        ui->textOutputDirectory->setText(outputfile);
    }
}

void MainWindow::on_checkEncodeText_toggled(bool checked) {
    if(checked) {
        ui->btnGoText->setText("Encode");
    }
}

void MainWindow::on_checkDecodeText_toggled(bool checked) {
    if(checked) {
        ui->btnGoText->setText("Decode");
    }
}

void MainWindow::on_btnGoText_clicked() {
    QByteArray ba;
    ba.append(ui->textInput->toPlainText());
    if(ui->checkEncodeText->isChecked()) {
        ui->textOutput->setPlainText(ba.toBase64());
    }
    else {
        ui->textOutput->setPlainText(ba.fromBase64(ba));
    }
}

void MainWindow::on_btnClearText_clicked() {
    ui->textInput->clear();
    ui->textOutput->clear();
}

void MainWindow::SetControlsActive(bool m) {
    //ui->btnGoFile->setEnabled(m);
    ui->btnAddFile->setEnabled(m);
    ui->btnClearList->setEnabled(m);
    ui->btnDeleteFile->setEnabled(m);
    ui->btnPickDir->setEnabled(m);
    ui->textOutputDirectory->setEnabled(m);
    ui->checkDecode->setEnabled(m);
    ui->checkEncode->setEnabled(m);
}

void MainWindow::on_btnAbout_clicked() {
    QString html;
    html = "<p><b style=\"font-size: 14pt\">"+QString(APPNAME)+"</b> "+QString(APPVER)+"<br>\n";
    html.append("&copy;2016-2018 <a href=\"https://www.matthewhipkin.co.uk\" style=\"color: #FF0000\">Matthew Hipkin</a><br>\n");
    html.append("<p>A base64 file encoder/decoder.</p>");
    html.append("<p><a href=\"https://twitter.com/hippy2094\"><img src=\":/images/logo_twitter_25px.png\"></a> <a href=\"https://sourceforge.net/projects/base64-binary/\"><img src=\":/images/sourceforge_logo_small.png\"></a> <a href=\"https://qt.io\"><img src=\":/images/Built_with_Qt_RGB_logo.png\"></p>");
    //html.append("<p><a href=\"https://qt.io\"><img src=\":/images/Built_with_Qt_RGB_logo.png\"></p>");
    QMessageBox::about(this,"About "+QString(APPNAME),html);
}

void MainWindow::CheckUpdates() {
    QString ua;
    ua = "Mozilla/5.0 (compatible; "+GetOS()+"; "+APPNAME+" "+APPVER+" ("+QString::number(CURRVER)+"))";
    QNetworkAccessManager nam;
    QUrl url("http://www.matthewhipkin.co.uk/bin64.txt");
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent",QByteArray(ua.toStdString().c_str()));
    QNetworkReply* reply = nam.get(req);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray bytes = reply->readAll();
    QString t = QString::fromUtf8(bytes);
    if(t.trimmed().toInt() > CURRVER) {
        labelUpdate = new QLabel(this);
        labelUpdate->setText("<p>A new version is available, <a href=\"https://www.matthewhipkin.co.uk\">click here</a> to get it!");
        labelUpdate->setVisible(true);
        labelUpdate->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(255, 204, 204, 255), stop:1 rgba(255, 255, 255, 255)); }");
        labelUpdate->setTextInteractionFlags(Qt::TextBrowserInteraction);
        labelUpdate->setOpenExternalLinks(true);
        ui->centralWidget->layout()->addWidget(labelUpdate);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    e->accept();
}

void MainWindow::dropEvent(QDropEvent *e) {
    foreach(QUrl url, e->mimeData()->urls()) {
        QString filename = url.toLocalFile();
        ui->listFiles->addItem(filename);
    }
}

void MainWindow::on_checkImgTag_clicked() {
    if(ui->checkImgTag->isChecked()) {
        ui->label->setText("Output filename");
        if(ui->checkDecode->isChecked()) {
            ui->checkDecode->setChecked(false);
            ui->checkEncode->setChecked(true);
        }
        ui->checkDecode->setEnabled(false);
    }
    else {
        ui->label->setText("Output directory");
        ui->checkDecode->setEnabled(true);
    }
}
