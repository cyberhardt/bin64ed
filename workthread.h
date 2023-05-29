#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>

enum WorkMode { wmEncode, wmDecode, wmImage };

class WorkThread : public QObject {
    Q_OBJECT
public:
    //explicit WorkThread(QObject *parent = nullptr);
    WorkThread();
    ~WorkThread();
    void SetMode(WorkMode mode);
    void SetOutputPath(QString path);
    void DoStop();
    void AddFile(QString filename);

signals:
    void Finished();
    void UpdateProgress(qint64 Position, qint64 Size);
    void FileComplete(int index, int count);

public slots:
    void Process();

private:
    WorkMode FMode;
    QString FOutputPath;
    QStringList FFileList;
    bool FStop;
    void EncodeFile(QString inputFile, QString outputFile);
    void DecodeFile(QString inputFile, QString outputFile);

};

#endif // WORKTHREAD_H
