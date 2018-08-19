#include "pocket.h"


QVector2D Pocket::pos() const
{
    return m_pos;
}

double Pocket::radius() const
{
    return m_radius;
}

void Pocket::render(QPainter &painter, const QVector2D &offset) {
    
    QVector2D absolutePos = offset + m_pos;

    painter.setBrush(m_pocketBrush);
    painter.drawEllipse(absolutePos.toPointF(), m_radius, m_radius);

    // draw a cute little text indicating how many balls sunk
    std::string numSunkStr = std::to_string(m_sunk);
    QPen oldPen = painter.pen();
    painter.setPen(QColor("white"));
    QRectF tRect((absolutePos - QVector2D(m_radius, m_radius)).toPointF(), QSizeF(2*m_radius, 2*m_radius));
    painter.drawText(tRect, Qt::AlignCenter, QString(numSunkStr.c_str()));

    painter.setPen(oldPen);
}
