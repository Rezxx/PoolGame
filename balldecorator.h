#pragma once

#include "ball.h"
#include "utils.h"
#include "mouseeventable.h"

/**
 * @brief The BallDecorator class
 * This acts as a Decorator. Any methods not overriden can simply forward the requests
 *  onto the Class this is decorated
 */
class BallDecorator : public Ball {
protected:
    Ball* m_subBall;

public:
    BallDecorator(Ball* b) : m_subBall(b) {}
    virtual ~BallDecorator() { delete m_subBall; }

    // mess of forwarded requests
    // is this the downside of a decorator..?
    virtual Ball* clone() override = 0;
    virtual bool isCue() override{return m_subBall->isCue();}
    virtual void render(QPainter &painter, const QVector2D& offset) override { m_subBall->render(painter, offset); }
    virtual void translate(QVector2D vec) override { m_subBall->translate(vec); }
    virtual QVector2D getVelocity() const override{ return m_subBall->getVelocity(); }
    virtual void setVelocity(QVector2D v) override { m_subBall->setVelocity(v); }
    virtual void changeVelocity(const QVector2D& delta) override { m_subBall->changeVelocity(delta); }
    virtual void multiplyVelocity(const QVector2D& vel) override { m_subBall->multiplyVelocity(vel); }

    virtual double getMass() const override { return m_subBall->getMass(); }
    virtual int getRadius() const override { return m_subBall->getRadius(); }
    virtual QVector2D getPosition() const override { return m_subBall->getPosition(); }
    virtual void setPosition(QVector2D p) override { m_subBall->setPosition(p); }
    virtual bool applyBreak(const QVector2D& q, std::vector<Ball*>& b) override { return m_subBall->applyBreak(q,b); }
};

/**
 * @brief The CueBall class
 * This handles some mouse interactions, and can control the position/velocity of the ball
 * The ball will only be able to be controlled if the mouse click&drag event originated at
 * the position of the cue ball.
 */
class CueBall : public BallDecorator, public MouseEventable {
protected:
    QVector2D posToSave;
    bool m_save;
    QVector2D m_lastV;
    // keep track of where the mouse click started at
    QVector2D m_startMousePos;
    // and the end
    QVector2D m_endMousePos;
    // whether the drag is happening
    bool isDragging = false;

    // whether we can consider this ball as having stopped.
    inline bool isSubBallMoving() const { return m_subBall->getVelocity().length() > MovementEpsilon; }

public:
    CueBall(Ball* b) : BallDecorator(b), MouseEventable(this), posToSave(b->getPosition()) {
        m_subBall->setCue();
    }
    ~CueBall() {}

    /**
     * @return if the game need to be saved
     */
    bool save(){return m_save;}

    /**
     * @brief notSave called when the game has been saved once
     */
    void notSave(){m_save = false;}

    /**
     * @brief clone of current cue ball
     * @return new cue ball with same value
     */
    CueBall* clone() override;

    /**
     * @brief render - draw this ball and the drag indicator if applicable
     * @param painter - the brush to use to draw
     * @param offset - where our pos is relative to
     */
    void render(QPainter &painter, const QVector2D &offset) override;

public:
    /**
     * @brief mouseClickEvent - update where the start of the mouse drag is.
     *      Chooses not to draw IF the click is not within bounds
     * @param e - the mouse event caused by clicking
     */
    virtual void mouseClickEvent(QMouseEvent* e) override;
    
    /**
     * @brief mouseMoveEvent - update where the current end of the mouse drag is.
     *      Used when the mouse is moved, i.e. not released, but dragged.
     * @param e - the mouse event caused by clicking
     */
    virtual void mouseMoveEvent(QMouseEvent* e) override;

    /**
     * @brief mouseReleaseEvent - update where the end of the mouse drag is, & release the click
     *  This will update the ball velocity if drawn.
     * @param e - the mouse event caused by clicking
     */
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
};

class BallSparkleDecorator : public BallDecorator {
protected:
    // our particle that is drawn
    struct Sparkle {
        Sparkle( QPointF pos)
            :pos(pos){}
        Sparkle(const Sparkle& s):
            pos(s.pos), opacity(s.opacity){}
        // absolute position
        QPointF pos;
        double opacity = 1.0;
        double width = 5.0;
        double height = 5.0;
    };

    // how fast the opacity is faded per drawn frame.
    // yes, this is frame dependent.
    static constexpr double fadeRate = 0.01;
    // our particle effects that will be drawn per frame
    std::vector<Sparkle> m_sparklePositions;
public:
    BallSparkleDecorator(Ball* b) : BallDecorator(b) {}

    /**
     * @brief clone of current sparkle ball
     * @return new sparkle ball with same value
     */
    BallSparkleDecorator* clone() override;

    /**
     * @brief render - draw the underlying ball and also the sparkles
     * @param painter - the brush to use to draw
     * @param offset - the offset that this ball is from the origin
     */
    void render(QPainter &painter, const QVector2D &offset) override;
};

class BallSmashDecorator : public BallDecorator {
protected:
    struct Crumb {
        Crumb(QPointF cPos,double width,double height,QVector2D dir, double opacity = 0)
            :pos(cPos),width(width),height(height),dir(dir),opacity(opacity){}
        Crumb(const Crumb& s):
            pos(s.pos), width(s.width), height(s.height), dir(s.dir), opacity(s.opacity){}

        // absolute position (from origin)
        QPointF pos;
        double width = 5.0;
        double height = 5.0;
        // particle tween direction
        QVector2D dir;
        double opacity = 1.0;
    };
    // how often they fade per frame
    static constexpr double fadeRate = 0.01;
    // rate of escape
    static constexpr double moveRate = 0.3;
    std::vector<Crumb> m_crumbs;

    void addCrumbs(QPointF cPos);
public:
    BallSmashDecorator(Ball* b) : BallDecorator(b) {}

    /**
     * @brief clone of current smash ball
     * @return new smash ball with same value
     */
    BallSmashDecorator* clone() override;

    /**
     * @brief changeVelocity - set the velocity of the ball, as well as generate particles (if applicable)
     * @param delta - the change in velocity
     */
    virtual void changeVelocity(const QVector2D& delta) override;

    /**
     * @brief multiplyVelocity - mul the velocity, as well as generate particles, if direction changes.
     * @param vel
     */
    virtual void multiplyVelocity(const QVector2D& vel) override {
        m_subBall->multiplyVelocity(vel);
        if (vel.x() < 0 || vel.y() < 0) addCrumbs(m_subBall->getPosition().toPointF());
    }

    /**
     * @brief render - draw the ball, the smash particles, as well as update the particle effects positions
     *  yes. we animate in the render function! ):<
     * @param painter - the brush to use to draw
     * @param offset - the offset from the window that this ball's pos is.
     */
    virtual void render(QPainter &painter, const QVector2D &offset) override;
};
