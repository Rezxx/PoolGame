#include "ball.h"
#include <iostream>

void StageOneBall::render(QPainter &painter, const QVector2D& offset) {
    // use our colour
    painter.setBrush(m_brush);
    // circle centered
    painter.drawEllipse((offset + m_pos).toPointF(), m_radius, m_radius);
}

Ball* CompositeBall::clone(){
    CompositeBall* ball = new CompositeBall(*this);

    //clone all the childrean balls as well
    for(int i = 0; i < m_children.size(); i++){
        ball->addChild(m_children.at(i)->clone());
    }
    return ball;
}

void CompositeBall::render(QPainter &painter, const QVector2D& offset) {
    recursiveRender(painter, offset);
}

void CompositeBall::recursiveRender(QPainter &painter, const QVector2D &offset) {
    // use our colour
    painter.setBrush(m_brush);

    // circle centered, plus offset
    painter.drawEllipse((offset + m_pos).toPointF(), m_radius, m_radius);

    // render children potentially
    if (m_renderChildren) for (Ball* b : m_children) b->render(painter, offset + m_pos);
}

bool CompositeBall::applyBreak(const QVector2D &deltaV, std::vector<Ball *> &parentlist) {
    double energyOfCollision = m_mass*deltaV.lengthSquared();
    if (energyOfCollision >= m_strength) {
        if (m_children.empty()) return true;

        // undo the delta to find the precollision velocity
        QVector2D preCollisionVelocity = m_velocity - deltaV;
        double energyPerBall = energyOfCollision/m_children.size();
        QVector2D pointOfCollision((-deltaV.normalized())*m_radius);
        // explode balls away from point of impact
        for (Ball* b : m_children) {
            b->setVelocity(preCollisionVelocity +
                           sqrt(energyPerBall/b->getMass())*
                           (b->getPosition()-pointOfCollision).normalized());
            // move the ball to be absolute
            b->translate(m_pos);
            parentlist.push_back(b);
        }
        return true;
    }
    return false;
}
