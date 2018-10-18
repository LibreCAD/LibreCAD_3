#pragma once

#include <QString>

class ItemStyle {
    public:
        ItemStyle(QString iniFile, QString grp);
        ~ItemStyle() = default;

        QString getStyle();

    private:
        QString filePath;
        QString group;

        QString getKey();
        QString getDefKey();
};