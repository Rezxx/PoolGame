#pragma once
#include "table.h"
#include "balldecorator.h"
/**
 * @brief The Strategy class defines the interface for different strategies to run the game
 */
class Strategy
{
public:
    Strategy(std::vector<Ball*>* balls, std::vector<Pocket*>* pockets): m_balls(balls), m_pockets(pockets){}
    ~Strategy(){}

    /**
     * @brief update contains additional calculation for the game in each animation
     */
    virtual void update() = 0;

    /**
     * @brief render additional graphics for the game
     */
    virtual void render(QPainter& painter) = 0;

    /**
     * @return a copy of strategy with current type
     */
    virtual Strategy* clone(std::vector<Ball*>* balls, std::vector<Pocket*>* pockets) = 0;

    /**
     * @return other type of strategy with current setup
     */
    virtual Strategy* switchMode() = 0;

protected:
    std::vector<Ball*>* m_balls;
    std::vector<Pocket*>* m_pockets;
};

/**
 * @brief The NoStrategy class is the vanila game with no additional feature
 */
class NoStrategy : public Strategy{
public:
    NoStrategy(std::vector<Ball*>* balls, std::vector<Pocket*>* pockets): Strategy(balls, pockets){}
    void update() override{}
    void render(QPainter&) override{}
    Strategy* clone(std::vector<Ball*>* balls, std::vector<Pocket*>* pockets) override {return new NoStrategy(balls, pockets);}
    Strategy* switchMode() override;
};

/**
 * @brief The AidStrategy class provides the feature of constantly finding the best angle to shoot the cue ball and visulising the
 * path of shooting
 */
class AidStrategy : public Strategy{
public:
    AidStrategy(std::vector<Ball*>* balls, std::vector<Pocket*>* pockets): Strategy(balls, pockets){}
    /**
     * @brief calculates the angle of chooting the cue ball
     */
    void update() override;

    /**
     * @brief render the path of shooting
     */
    void render(QPainter& painter) override;
    Strategy* clone(std::vector<Ball*>* balls, std::vector<Pocket*>* pockets) override {return new AidStrategy(balls, pockets);}
    Strategy* switchMode() override;
private:
    /**
     * @brief calculateC2 - the desired position for cue ball after shooting
     * @param ballA - the target ball to sink
     * @param cue - the cue ball
     * @param pocket - the target pocket to sink target ball
     * @return the desired position of cue ball
     */
    QVector2D calculateC2(Ball *ballA, Ball *cue, Pocket *pocket);

    /**
     * @brief findPocket - finds the best pocket to sink target ball
     * @param ballA - the target ball to sink
     * @param cue - the cue ball
     * @return the pocket with samllest angle adjustment of pocket - target ball - cue ball
     */
    Pocket *findPocket(Ball *ballA, Ball *cue);

    /**
     * @brief calculateDot - calculate CA dot product AP
     * @param ballA - A
     * @param cue - C
     * @param pocket - P
     * @return the dot product
     */
    double calculateDot(Ball *ballA, Ball *cue, Pocket *pocket);

    /**
     * @brief checkBall - check the if there's any ball or pocket intersect CA line
     * @param ballA - A
     * @param cue - C
     * @return true if there's no any intersection, false otherwise
     */
    bool checkBall(Ball *ballA, Ball *cue);

    /**
     * @brief checkPocket - check if there's any ball intersect AP line
     * @param ballA - A
     * @param pocket - P
     * @return true if there's no any intersection, false otherwise
     */
    bool checkPocket(Ball *ballA, Pocket *pocket);

    /**
     * @brief minimum_distance find the minumum distance from Point P to line VW
     */
    float minimum_distance(QVector2D v, QVector2D w, QVector2D p);

    /**
     * @brief findCue - find the cue ball from all the balls
     * @return the cue ball
     */
    CueBall *findCue();

private:
    QVector2D toCue; // the desired cue position after shooting

    Pocket* toPocket = 0; //the wanted pocket to sink the ball
};
