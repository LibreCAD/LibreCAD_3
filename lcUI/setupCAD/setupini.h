#pragma once

#include <QString>

class SetupIni {
    public:
        SetupIni(const QString& iniFile);
        ~SetupIni() = default;

        QString getFilePath();

    private:
        QString iniFilePath; //ini file name and file path

        void checkDir();
        void checkFile();
};
