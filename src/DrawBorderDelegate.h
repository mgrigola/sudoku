#ifndef DRAWBORDERDELEGATE_H
#define DRAWBORDERDELEGATE_H

#include <QItemDelegate>
#include <QPainter>

class DrawBorderDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit DrawBorderDelegate(QObject *parent = 0);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

 /*
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;


private slots:
    void commitAndCloseEditor();
*/
};

#endif // DRAWBORDERDELEGATE_H
