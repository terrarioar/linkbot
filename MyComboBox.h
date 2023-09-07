#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>
#include <QMouseEvent>
#include "mainwindow.h"


class MyComboBox : public QComboBox
{
    Q_OBJECT    
public:
    explicit MyComboBox(QWidget* parent = nullptr);
    ~MyComboBox();
protected:
    virtual void mousePressEvent(QMouseEvent* e); 

signals:
    void mouseSingleClickd(); 
};

#endif // MYCOMBOBOX_H
