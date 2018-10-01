#include "itemStyle.h"
#include "setupini.h"
#include <QString>
#include <QSettings>
#include <QStyleFactory>

QString ItemStyle::getKey() { //get key for Item style
    QSettings setting(ItemStyle::filePath,QSettings::IniFormat); // INI file format set
    setting.beginGroup(ItemStyle::group);
    QString str = setting.value("Style").toString();
    setting.endGroup();
    return str;
}

QString ItemStyle::getDefKey() {
    QSettings setting(ItemStyle::filePath,QSettings::IniFormat); // INI file format set
    setting.beginGroup(ItemStyle::group);
    QString str = setting.value("DefaultStyle").toString();
    setting.endGroup();
    return str;
}

QString ItemStyle::getStyle() {
    QString itemKey = ItemStyle::getKey();
    QString itemDefKey = ItemStyle::getDefKey();
    // set style
    QList<QString> keyList = QStyleFactory::keys();
    if(keyList.indexOf(itemKey) < 0) { //When Style no match the item
        itemKey = "Default";
    }
    // when stylename "Default"
    if (itemKey == "Default") {
        itemKey = ItemStyle::getDefKey();
        if(keyList.indexOf(itemKey) < 0) { //When DefaultStyle no match the item
            itemKey = QStyleFactory::keys().first();
        }
    }
    return itemKey;
}

ItemStyle::ItemStyle(QString iniFile, QString grp) {
    // set file/path and groupname
    ItemStyle::filePath = iniFile;
    ItemStyle::group = grp;
    // check style item
    QSettings setting(ItemStyle::filePath, QSettings::IniFormat); // INI file format set
    setting.beginGroup(ItemStyle::group);
    // make style item when no style item
    if (setting.value("DefaultStyle").toString() == "") { // make default style
        setting.setValue("DefaultStyle", QStyleFactory::keys().first());
    }
    if (setting.value("Style").toString() == "") { // make using style
        setting.setValue("Style", "Default");
    }
    setting.endGroup();
}

ItemStyle::~ItemStyle() { }