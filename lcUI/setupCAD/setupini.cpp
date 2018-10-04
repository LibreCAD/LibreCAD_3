#include "itemStyle.h"
#include "setupini.h"
#include <QFileInfo>
#include <QDir>

//make directory when no directory
void SetupIni::checkDir() {
    QFileInfo fileInfo(iniFilePath);

    if (!fileInfo.isDir()) { 
        QDir dir;
        dir.mkdir(fileInfo.path());
    }
}

//make file when no file
void SetupIni::checkFile() {
    QFileInfo fileInfo(iniFilePath);

    if (!fileInfo.isFile()) { 
        // Create a new file
        QFile file(fileInfo.filePath());
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.close();
    }
}


QString SetupIni::getFilePath() {
    return iniFilePath;
}

SetupIni::SetupIni(QString iniFile) {
    //set file/path and key_group
    iniFilePath = QDir::homePath() + iniFile;

    // check file/path
    checkDir();
    checkFile();
}
