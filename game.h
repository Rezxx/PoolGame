#pragma once
#include <QJsonObject>
#include <functional>
#include <QMouseEvent>

#include "abstractstagefactory.h"

#include "table.h"
#include "ball.h"
#include "balldecorator.h"
#include "utils.h"
#include "visiter.h"
#include "strategy.h"

class Game {
    //if the game needs to be saved in next update
    bool m_save;
    //choose strategy between no visual aid and visual aid
    Strategy* m_strategy;
    //indicates if the game is in stage three
    bool m_stageThree;

    std::vector<Ball*>* m_balls;
    Table* m_table;
    // screenshake stuff
    QVector2D m_screenshake;
    double m_shakeRadius = 0.0;
    double m_shakeAngle = 0;
    static constexpr double SCREENSHAKEDIST = 10.0;

    /* increase the amount of screen shake */
    void incrementShake(double amount=SCREENSHAKEDIST) { m_shakeRadius += amount; }
private:
    // store the functions that get scanned through whenever a mouse event happens
    MouseEventable::EventQueue m_mouseEventFunctions;

    /**
     * @brief updateShake - update the screenshake radius (make it smaller)
     * @param dt - the timestep change
     */
    void updateShake(double dt);

    /**
     * @brief getPockets utalise visiter to get pocket from table
     * @return a vector of pockets
     */
    std::vector<Pocket*>* getPockets();

    /**
     * @brief generateBall - generates a random ball
     * @param max_x - the width of the table
     * @param max_y - the height of the table
     * @return a ball located within the table
     */
    CompositeBall *generateBall(int max_x, int max_y);

    /**
     * @brief generateSubBall - generate a random ball inside of other ball
     * @param radius - the containing ball's radius
     * @return a ball within the given ball
     */
    CompositeBall *generateSubBall(int radius);

    /**
     * @brief decrateBall - add random decoration to the ball
     * @return a ball decorated with crumb, sparkle or nothing
     */
    Ball *decrateBall(Ball *ball);
public:
    ~Game();
    Game(std::vector<Ball*>* balls, Table* table) :
        m_save(false), m_balls(balls), m_table(table), m_stageThree(false){
        m_strategy = new NoStrategy(m_balls,getPockets()); //default with no aid
    }
    //copy constructor
    Game(Game& game);

    /**
     * @brief switchMode - switches the current game mode between AI aid or not
     */
    void switchMode(){m_strategy = m_strategy->switchMode();}

    /**
     * @return if the game needs to be saved
     */
    bool toSave(){return m_save;}

    /**
     * @brief notSave - called when the game has been saved
     */
    void notSave(){m_save = false;}

    /**
     * @brief clone a new game with same setup
     */
    Game* clone();

    /**
     * @brief findCue - goes through all the balls and find the cue ball
     * @return the cue ball
     */
    CueBall* findCue();

    /**
     * @brief addRandomBall - add a random ball to the game
     */
    void addRandomBall();

    /**
     * @brief setStageThree - set stageThree to add addtional features
     */
    void setStageThree(){m_stageThree = true;}

    /**
     * @return if the game is in stage three
     */
    bool isStageThree() const{return m_stageThree;}

    /**
     * @brief Draws all owned objects to the screen (balls and table)
     * @param painter - qtpainter to blit to screen with
     */
    void render(QPainter& painter);

    /**
     * @brief Updates the positions of all objects within, based on how much time has changed
     * @param dt - time elapsed since last frame in seconds
     */
    void animate(double dt);

    /* how large the window's width should at least be */
    int getMinimumWidth() const { return m_table->getWidth(); }
    /* how large the window's height should at least be */
    int getMinimumHeight() const { return m_table->getHeight(); }
    /**
     * @brief resolveCollision - modify the ball's velocity if it is colliding with the table
     * @param table - the table to be bounds checked
     * @param ball - the ball to move
     * @return velocity - the change of velocity that a ball underwent
     */
    QVector2D resolveCollision(const Table* table, Ball* ball);
    /**
     * @brief resolveCollision - resolve both ball's velocity whether these balls collide
     * @param ballA - first ball
     * @param ballB - second ball
     * @param pair<deltaVelocityA, deltaVelocityB> - the change of velocities for each ball
     */
    std::pair<QVector2D, QVector2D> resolveCollision(Ball* ballA, Ball* ballB);

    /**
     * @brief isColliding - returns whether two balls are touching each other
     * @param ballA
     * @param ballB
     * @return whether the two balls are touching each other
     */
    bool isColliding(const Ball* ballA, const Ball* ballB) {
        QVector2D collisionVector = ballB->getPosition() - ballA->getPosition();
        return !(collisionVector.length() > ballA->getRadius() + ballB->getRadius());
    }

    /**
     * @brief addMouseFunctions - append all of the specified functions to be
     *  our eventqueue - these will be cycled through onclick, etc
     * @param fns
     */
    void addMouseFunctions(MouseEventable::EventQueue fns) {
        std::copy(fns.begin(), fns.end(), std::back_inserter(m_mouseEventFunctions));
    }

    /**
     * @brief getEventFns - get all of our event functions (mouseclicksfns, etc)
     * @return event queue of event functions
     */
    MouseEventable::EventQueue& getEventFns() { return m_mouseEventFunctions; }

};
