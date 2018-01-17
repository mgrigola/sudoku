#ifndef WIDGETLEGENDLINES_H
#define WIDGETLEGENDLINES_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QPaintDevice>
#include <QPainter>
#include <QLinearGradient>
#include <QPalette>

#include "SudokuTable.h"

class WidgetLegendLines : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetLegendLines(QWidget *parent = 0);
    void paintEvent(QPaintEvent*);

signals:

public slots:
};

#endif // WIDGETLEGENDLINES_H
