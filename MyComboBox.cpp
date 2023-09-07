#include "MyComboBox.h"

using namespace Ui;

MyComboBox::MyComboBox(QWidget* parent) :QComboBox(parent)
{

}

MyComboBox::~MyComboBox()
{

}

void MyComboBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit mouseSingleClickd();
    }

    QComboBox::mousePressEvent(event);
}