#include "DrawBorderDelegate.h"

DrawBorderDelegate::DrawBorderDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void DrawBorderDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    const QRect rect(option.rect);
    painter->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));

    if (index.column()%3 == 0 )
        painter->drawLine(rect.topLeft(),rect.bottomLeft());
    if ((index.column()+1)%3 == 0 )
        painter->drawLine(rect.topRight(),rect.bottomRight());

    if ((index.row()+1)%3 == 0 )
        painter->drawLine(rect.bottomLeft(),rect.bottomRight());
    if ((index.row())%3 == 0 )
        painter->drawLine(rect.topLeft(),rect.topRight());

    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    painter->drawRect(rect);

    QItemDelegate::paint( painter, option, index );

    return;
}

/*

void DrawBorderDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);
    if (edit) {
        model->setData(index, edit);
        return;
    }
}



void DrawBorderDelegate::commitAndCloseEditor()
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
*/
