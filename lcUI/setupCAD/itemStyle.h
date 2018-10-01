/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ItemStyle.h
 * Author: yori
 *
 * Created on 2018/09/29, 0:04
 */

#ifndef ITEMSTYLE_H
#define ITEMSTYLE_H

#endif /* ITEMSTYLE_H */

#include <QString>

class ItemStyle {
private:
    QString filePath;
    QString group;
    QString getKey();
    QString getDefKey();
public:
    QString getStyle();

    ItemStyle(QString iniFile, QString grp);
    ~ItemStyle();
};