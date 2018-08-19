#pragma once

#include <QColor>
#include <QPainter>

#include "pocket.h"
#include "visiter.h"

class Ball;
class Visiter;

class Table {
protected:
    int m_width;
    int m_height;
    QBrush m_brush;
    double m_friction;
public:
    virtual ~Table() {}
    Table(int width, int height, QColor colour, double friction) :
        m_width(width), m_height(height),
        m_brush(colour), m_friction(friction) {}
    //copy constructor
    Table(Table& table):m_width(table.m_width), m_height(table.m_height),
        m_brush(table.m_brush), m_friction(table.m_friction){}

    /**
     * @return a copy of current table with same setup
     */
    virtual Table* clone() = 0;
    /**
     * @brief render - draw the table to screen using the specified painter
     * @param painter - painter to use
     */
    virtual void render(QPainter& painter, const QVector2D& offset) = 0;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    double getFriction() const { return m_friction; }

    virtual bool sinks(Ball*) { return false; }

    /**
     * @brief accept - take the visiter and let it access pockets
     * @param v - the visiter
     * @return - no pockets in stageone table
     */
    virtual std::vector<Pocket*>* accept(Visiter*){return NULL;}
};

class StageOneTable : public Table
{
public:
    StageOneTable(int width, int height, QColor colour, double friction) :
        Table(width, height, colour, friction) {}
    //copy constructor
    StageOneTable(StageOneTable& table):Table(table){}

    Table* clone() override{return new StageOneTable(*this);}

    /**
     * @brief render - draw the stageonetable to screen using the specified painter
     * @param painter - painter to use
     */
    void render(QPainter &painter, const QVector2D& offset) override;
};

class StageTwoTable : public Table {
protected:
    std::vector<Pocket*> m_pockets;

public:
    StageTwoTable(int width, int height, QColor colour, double friction) :
        Table(width, height, colour, friction) {}

    //copy constructor
    StageTwoTable(StageTwoTable& table);
    ~StageTwoTable();

    Table* clone() override{return new StageTwoTable(*this);}

    /**
     * @brief accept - take the visiter and let it access pockets
     * @param v - the visiter
     * @return - the pockets
     */
    std::vector<Pocket*>* accept(Visiter* ) override{return &m_pockets;}

    /**
     * @brief render - draw the stageonetable to screen using the specified painter
     * @param painter - painter to use
     */
    void render(QPainter &painter, const QVector2D& offset) override;

    // sinky winky ball
    virtual bool sinks(Ball* b) override;

    /* self explanatory */
    void addPocket(Pocket* p) { m_pockets.push_back(p); }
};
