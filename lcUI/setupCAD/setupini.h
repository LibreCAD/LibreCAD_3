/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   setupini.h
 * Author: yori
 *
 * Created on 2018/09/28, 14:21
 */

#ifndef SETUPINI_H
#define SETUPINI_H

#endif /* SETUPINI_H */

#include <QString>

class SetupIni {

private:
    QString iniFilePath; //ini file name and file path
    
    void checkDir();
    void checkFile();

public:
    QString getFilePath();

    SetupIni(QString iniFile);
    ~SetupIni();
};
