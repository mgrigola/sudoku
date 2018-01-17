#include "WidgetLegendLines.h"

WidgetLegendLines::WidgetLegendLines(QWidget *parent) : QWidget(parent)
{
    this->setGeometry(0,0,30,160);
    this->setMinimumHeight(160);
    this->setMinimumWidth(30);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void WidgetLegendLines::paintEvent(QPaintEvent*)
{
    QPainter painterLegend(this);

    QRect maxRect = this->geometry();
    int h = maxRect.height();
    int w = maxRect.width();
    int xLeft = w/6;
    int xRight = 5*w/6;
    int dH = h/4+1;
    int h0 = -h/8-1;

    QPalette pal = this->palette();
    QLinearGradient linearGrad(0,0,w,h);
    //QLinearGradient linearGrad(QPoint(0, 0), QPoint(w, h) );
    linearGrad.setColorAt(0, QColor(192,255,192));
    linearGrad.setColorAt(1, QColor(192,192,255));
    pal.setBrush(QPalette::Base, QBrush(linearGrad) );
    pal.setBrush(QPalette::Highlight, QBrush(linearGrad) );
    pal.setBrush(QPalette::Background, QBrush(linearGrad) );
//    pal.setBrush(QPalette::NoRole, QBrush(linearGrad) );
//    pal.setBrush(QPalette::Normal, QBrush(linearGrad) );

    this->setPalette(pal);
    //this->setBackgroundRole(QPalette::Background);
    this->setAutoFillBackground(true); //this means like "actually draw the background I just told you to draw!"!!

    //setBackgroundBrush(linearGrad);


    //QPen(sudokuTable->methodColors[1]
    QPen myPen(ColorMaps::methodColors[1], 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    //QPen myPen(MethodColorMap(1), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painterLegend.setPen(myPen);
//    QBrush brushLegend(sudokuTable->methodColors[1]);
//    painterLegend.setBrush(brushLegend);
    painterLegend.drawLine(xLeft,h0+dH,xRight,h0+dH);
    myPen.setColor(ColorMaps::methodColors[2]);
    //painterLegend.pen.setColor(Qt::magenta);
    painterLegend.setPen(myPen);
    //myPen.setColor(MethodColorMap(2));
    painterLegend.drawLine(xLeft,h0+2*dH,xRight,h0+2*dH);
    myPen.setColor(ColorMaps::methodColors[3]);
    painterLegend.setPen(myPen);
    //myPen.setColor(MethodColorMap(3));
    painterLegend.drawLine(xLeft,h0+3*dH,xRight,h0+3*dH);

    myPen.setColor(ColorMaps::methodColors[4]);
    painterLegend.setPen(myPen);
    //myPen.setColor(MethodColorMap(4));
    painterLegend.drawLine(xLeft,h0+4*dH,xRight,h0+4*dH);

    this->show();

}
