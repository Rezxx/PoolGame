#include "table.h"
#include "ball.h"
#include <iostream>

void StageOneTable::render(QPainter &painter, const QVector2D& offset) {
    // our table colour
    painter.setBrush(m_brush);
    // draw table
    painter.drawRect(offset.x(), offset.y(), this->getWidth(), this->getHeight());
}

void StageTwoTable::render(QPainter &painter, const QVector2D& offset) {
    // our table colour
    painter.setBrush(m_brush);
    // draw table
    painter.drawRect(offset.x(), offset.y(), this->getWidth(), this->getHeight());

    // render the pockets relative to this table
    for (Pocket* p : m_pockets) {
        p->render(painter, offset);
    }
}

StageTwoTable::StageTwoTable(StageTwoTable &table):Table(table){
    for(Pocket* pocket : table.m_pockets){
        m_pockets.push_back(new Pocket(*pocket));
    }
}

StageTwoTable::~StageTwoTable() {
    for (Pocket* p : m_pockets) delete p;
}

bool StageTwoTable::sinks(Ball *b) {
    QVector2D absPos = b->getPosition();
    double radius = b->getRadius();
    // check whether any pockets consumes this ball
    for (Pocket* p : m_pockets) {
        // you sunk my scrabbleship
        if (p->contains(absPos, radius)) {
            if(b->isCue()){
                Pocket* p2 = m_pockets.at(rand()% (m_pockets.size() - 1));
                while(p2 == p){
                    p2 = m_pockets.at(rand()% (m_pockets.size() - 1));
                }
                b->setPosition(p2->pos());
                QVector2D *v = new QVector2D(rand()%40 - 20, rand()%40 - 20);
                b->setVelocity((*v) *10);
                break;
            }else{
                p->incrementSunk();
                return true;
            }
        }
    }
    return false;
}
