#include "workthread.h"

WorkThread::WorkThread() {
    FStop = false;
}

WorkThread::~WorkThread() {

}

// http://www.cnblogs.com/zhaox583132460/p/3436942.html

void WorkThread::EncodeFile(QString inputFile, QString outputFile) {
    QFile inputStream(inputFile);
    QFile outputStream(outputFile);
    QTextStream out(&outputStream);
    int bufsize = 3 * 1024;
    QMimeDatabase db;
    if(inputStream.open(QIODevice::ReadOnly)) {
        outputStream.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        if(FMode == wmImage) {
            int c = 0;
            QString imgtext;
            QTextStream imgout(&imgtext);
            while(!inputStream.atEnd()) {
                if(FStop) break;
                QByteArray ind = inputStream.read(bufsize);
                imgout << ind.toBase64();
                if(c == 200 || inputStream.pos() == inputStream.size()) {
                    emit UpdateProgress(inputStream.pos(),inputStream.size());
                    c = 0;
                }
                c++;
            }
            QMimeType type = db.mimeTypeForFile(inputFile);
            out << "<img src=\"data:"+type.name()+";base64,"+imgtext+"\">\n";
            outputStream.close();
        }
        else {
            int c = 0;
            while(!inputStream.atEnd()) {
                if(FStop) break;
                QByteArray ind = inputStream.read(bufsize);
                out << ind.toBase64();
                if(c == 200 || inputStream.pos() == inputStream.size()) {
                    emit UpdateProgress(inputStream.pos(),inputStream.size());
                    c = 0;
                }
                c++;
            }
            outputStream.close();
        }
        inputStream.close();
    }
}

void WorkThread::DecodeFile(QString inputFile, QString outputFile) {
    QFile inputStream(inputFile);
    QFile outputStream(outputFile);
    int bufsize = 4 * 1024;
    if(inputStream.open(QIODevice::ReadOnly | QIODevice::Text)) {
        outputStream.open(QIODevice::WriteOnly);
        int c = 0;
        while(!inputStream.atEnd()) {
            if(FStop) break;
            QByteArray ind = inputStream.read(bufsize);
            outputStream.write(QByteArray::fromBase64(ind));
            if(c == 200 || inputStream.pos() == inputStream.size()) {
                emit UpdateProgress(inputStream.pos(),inputStream.size());
                c = 0;
            }
            c++;
        }
        outputStream.close();
        inputStream.close();
    }
}

void WorkThread::Process() {
    int i;
    for(i = 0; i < FFileList.count(); i++) {
        if(FStop) break;
        QFileInfo fi(FFileList.at(i));
        if(!FOutputPath.endsWith("/") && !FOutputPath.endsWith("\\")) {
            if(FMode != wmImage) FOutputPath.append("/");
        }
        if(FMode == wmImage) {
            EncodeFile(FFileList.at(i),FOutputPath);
        }
        else {
            if(FMode == wmEncode) {
                QString OutputFileName = FOutputPath + fi.fileName() + ".enc";
                EncodeFile(FFileList.at(i),OutputFileName);
            }
            else {
                QString OutputFileName;
                if(fi.fileName().endsWith(".enc")) {
                    OutputFileName = FOutputPath + fi.fileName().replace(".enc","");
                }
                else {
                    OutputFileName = FOutputPath + fi.fileName() + ".decoded";
                }
                DecodeFile(FFileList.at(i),OutputFileName);
            }
        }
        emit FileComplete((i+1),FFileList.count());
    }
    emit Finished();
}

void WorkThread::SetMode(WorkMode mode) {
    FMode = mode;
}

void WorkThread::SetOutputPath(QString path) {
    FOutputPath = path;
}

void WorkThread::AddFile(QString filename) {
    FFileList << filename;
}

void WorkThread::DoStop() {
    FStop = true;
}
