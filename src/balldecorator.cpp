#include "balldecorator.h"


CueBall *CueBall::clone(){
    Ball* sub = m_subBall->clone();
    sub->setVelocity(*new QVector2D()); //set cue velocity to 0
    sub->setPosition(posToSave); //set position to last saved point
    return new CueBall(sub);
}

void CueBall::render(QPainter &painter, const QVector2D &offset) {
    m_subBall->render(painter, offset);
    // stop drawing the line if we're moving at all
    if (isSubBallMoving()) isDragging = false;
    if (isDragging) {
        painter.drawLine(m_startMousePos.toPointF(), m_endMousePos.toPointF());
    }
}

void CueBall::mouseClickEvent(QMouseEvent* e) {

    QVector2D p = QVector2D(e->pos());

    // don't allow movement if moving
    if (isSubBallMoving()) {
        // can't be too sure
        isDragging = false;
        return;
    }

    m_startMousePos = p;
    m_endMousePos = p;

    // note this treats it as absolute distance
    // check whether the click isn't on the ball
    if (m_startMousePos.distanceToPoint(m_subBall->getPosition()) > m_subBall->getRadius()) return;

    isDragging = true;

}

void CueBall::mouseMoveEvent(QMouseEvent* e) {
    QVector2D p = QVector2D(e->pos());

    if (isSubBallMoving()) {
        isDragging = false;
        return;
    }
    m_endMousePos = p;
}

void CueBall::mouseReleaseEvent(QMouseEvent* e) {

    QVector2D p = QVector2D(e->pos());
    
    // only draw & move if we're allowing the draw action to go ahead
    if (isDragging) {
        // velocity is the vector that the mouse drew
        m_save = true;
        posToSave = getPosition();
        m_endMousePos = p;
        QVector2D resultingVel = m_endMousePos - posToSave;
        isDragging = false;
        // update ball vel
        m_subBall->changeVelocity(resultingVel);
    }
}

BallSparkleDecorator *BallSparkleDecorator::clone(){
    BallSparkleDecorator* ball = new BallSparkleDecorator(m_subBall->clone());
    //copy the sparkles too
    for(Sparkle s : m_sparklePositions){
        ball->m_sparklePositions.push_back(Sparkle(s));
    }
    return ball;
}

void BallSparkleDecorator::render(QPainter &painter, const QVector2D &offset) {
    m_subBall->render(painter, offset);

    // 1/10 chance to make a new sparkle (must not be moving)
    if (rand() % 10 == 0 && m_subBall->getVelocity().length() >= MovementEpsilon) {
        m_sparklePositions.push_back(Sparkle(m_subBall->getPosition().toPointF()));
    }

    for (ssize_t i = 0; i < (ssize_t) m_sparklePositions.size(); ++i) {
        Sparkle& s = m_sparklePositions.at(i);
        painter.setBrush(QBrush(QColor("yellow")));
        painter.setOpacity(s.opacity);
        // 5x5 mini rect randomly oscillating
        QRectF r(offset.x() + s.pos.x() + (rand()%6)-3,
                 offset.y() + s.pos.y() + (rand()%6)-3, s.width , s.height);
        painter.drawRect(r);

        s.opacity -= fadeRate;
        // remove when non-visible
        if (s.opacity <= 0) {
            m_sparklePositions.erase(m_sparklePositions.begin() + i);
            // so we don't skip positions
            --i;
        }
        painter.setOpacity(1);
    }
}

void BallSmashDecorator::addCrumbs(QPointF cPos) {
    size_t numAdding = rand() % 10;
    for (size_t i = 0; i < numAdding; ++i) {
        double width = (rand()%100)/20.0;
        double height = (rand()%100)/20.0;
        QVector2D dir(rand()%10-5, rand()%10-5);
        m_crumbs.push_back(Crumb(cPos, width, height, dir));
    }
}

BallSmashDecorator *BallSmashDecorator::clone(){
    BallSmashDecorator* ball = new BallSmashDecorator(m_subBall->clone());
    //copy the crumbs too
    for(Crumb c : m_crumbs){
        ball->m_crumbs.push_back( Crumb(c));
    }
    return ball;
}

void BallSmashDecorator::changeVelocity(const QVector2D &delta) {
    // whenever our velocity changes a lot, we should add some particles
    QVector2D preVel = m_subBall->getVelocity();
    m_subBall->changeVelocity(delta);
    double lenChange = fabs((preVel - m_subBall->getVelocity()).length());
    if (lenChange > 3.0) addCrumbs(m_subBall->getPosition().toPointF());
}

void BallSmashDecorator::render(QPainter &painter, const QVector2D &offset) {
    m_subBall->render(painter, offset);

    // animating in render!? yeah, this should be refactored..?
    // perhaps this may be a good idea to mention in the report, and
    // refactor for assgn3 so that the animation functions are now member functions of balls
    // this enables the simplification of the Game::animate fn, as well as physics defining decorators

    // draw the crummies
    for (ssize_t i = 0; i < (ssize_t) m_crumbs.size(); ++i) {
        // move the crummie
        Crumb& c = m_crumbs.at(i);
        c.pos += (c.dir*moveRate).toPointF();

        painter.setBrush(QBrush(QColor("gray")));
        painter.setOpacity(c.opacity);
        // our lil crumb object
        QRectF r(offset.x() + c.pos.x(),
                 offset.y() + c.pos.y(), c.width, c.height);
        painter.drawRect(r);

        c.opacity -= fadeRate;
        // remove when non-visible particles
        if (c.opacity <= 0) {
            m_crumbs.erase(m_crumbs.begin() + i);
            // so we don't skip positions
            --i;
        }
        // reset to opaque
        painter.setOpacity(1);
    }
}
