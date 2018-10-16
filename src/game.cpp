#include "game.h"
#include "utils.h"

#include <QJsonArray>
#include <stdexcept>
#include <cmath>
#include <exception>
#include <iostream>

Game::~Game() {
    // cleanup ya boi
    for (auto b : *m_balls) delete b;
    delete m_balls;
    delete m_table;
}

Game::Game(Game &game): m_save(false){
    m_table = game.m_table->clone();
    m_balls = new std::vector<Ball*>();
    m_stageThree = game.m_stageThree;
    for(int i = 0; i< game.m_balls->size(); i++){
        m_balls->push_back(game.m_balls->at(i)->clone());
    }
    CueBall* cue = findCue();
    addMouseFunctions(cue->getEvents()); //register the mouse events to the game
    m_strategy = game.m_strategy->clone(m_balls, getPockets());
}

Game *Game::clone(){
    Game* game = new Game(*this);
    return game;
}

CueBall *Game::findCue(){
    for (int i = 0; i < m_balls->size();i++) {
        Ball* ball = m_balls->at(i);
        if(ball->isCue()){
            CueBall* cue = static_cast<CueBall*> (ball);
            return cue;
        }
    }
    return NULL;
}

void Game::addRandomBall()
{
    CompositeBall* ball = generateBall(m_table->getWidth(), m_table->getHeight());

    bool overlap = false;
    //chekc no overlap with other balls
    for(Ball* other : *m_balls){
        double distance = other->getPosition().distanceToPoint(ball->getPosition());
        double min_dis = other->getRadius() + ball->getRadius();
        if(min_dis > distance){
            overlap = true;
            break;
        }
    }
    //check no overlap with pockets
    for(Pocket* pocket : *getPockets()){
        double distance = pocket->pos().distanceToPoint(ball->getPosition());
        double min_dis = pocket->radius() + ball->getRadius();
        if(min_dis > distance){
            overlap = true;
            break;
        }
    }

    if(overlap){
        delete ball;
        addRandomBall();//try it again
    }else{
        int sub_ball_num = rand()%3; // number of childrean balls varies from 0 to 2
        for(int i = 0; i < sub_ball_num; i++){
            Ball* b = generateSubBall(ball->getRadius());
            ball->addChild(decrateBall(b));
        }
        m_balls->push_back(decrateBall(ball));
    }
}

Ball* Game::decrateBall(Ball* ball){
    int num = rand() % 3;
    if(num == 0){
        ball = new BallSparkleDecorator(ball);//add sparkle
    }else if(num == 1){
        ball = new BallSmashDecorator(ball);//add crumbs
    }
    //when num == 2 no decoration :<
    return ball;
}

CompositeBall* Game::generateBall(int max_x, int max_y){
    QColor colour(rand()%255,rand()%255,rand()%255);
    int radius = rand() % 20 + 10;
    QVector2D position = QVector2D(rand()%(max_x - 2*radius) + radius, rand()%(max_y - 2* radius) + radius);
    QVector2D velocity = QVector2D();
    double mass = rand()%4 + 1;
    double strength = rand()% 50000 + 100;
    return new CompositeBall(colour,position,velocity,mass,radius,strength);
}

CompositeBall* Game::generateSubBall(int radius){
    QColor colour(rand()%255,rand()%255,rand()%255);
    int b_radius = rand() % (radius - 5) + 5;
    int r_limit = radius - b_radius;
    QVector2D position = QVector2D(rand()%(2*r_limit) - r_limit, rand() % (2*r_limit) - r_limit);
    QVector2D velocity = QVector2D();
    double mass = rand()%4 + 1;

    double strength = rand()% 50000 + 100;
    return new CompositeBall(colour,position,velocity,mass,b_radius,strength);
}

void Game::render(QPainter &painter) {
    // table is rendered first, as its the lowest
    m_table->render(painter, m_screenshake);

    // then render all the balls
    for (Ball* b : *m_balls){
        b->render(painter, m_screenshake);
    }
    if(m_stageThree){
        m_strategy->render(painter);
    }
}

void Game::animate(double dt) {
    if(m_stageThree){
        //check for saving game
        CueBall* cue = findCue();
        //when the cue ball exsits and want to save the game
        if(cue!= 0 && cue->save()){
            m_save = true;
            cue->notSave();// once is enough
        }
        m_strategy->update();
    }

    // keep track of the removed balls (they're set to nullptr during loop)
    // clean up afterwards
    std::vector<Ball*> toBeRemoved;
    // add these balls to the list after we finish
    std::vector<Ball*> toBeAdded;

    // (test) collide the ball with each other ball exactly once
    // to achieve this, balls only check collisions with balls "after them"
    for (auto it = m_balls->begin(); it != m_balls->end(); ++it) {
        Ball* ballA = *it;
        if (ballA == nullptr) continue;
        // correct ball velocity if colliding with table
        QVector2D tableBallDeltaV = resolveCollision(m_table, ballA);
        // test and resolve breakages with balls bouncing off table
        if (ballA->applyBreak(tableBallDeltaV, toBeAdded)) {
            // mark this ball to be deleted
            toBeRemoved.push_back(ballA);
            incrementShake();
            // nullify this ball
            *it = nullptr;

            continue;
        }

        // check whether ball should be swallowed
        if (m_table->sinks(ballA)) {
            // defer swallowing until later (messes iterators otherwise)
            toBeRemoved.push_back(ballA);
            // nullify this ball
            *it = nullptr;
            continue;
        }

        // check collision with all later balls
        for (auto nestedIt = it + 1; nestedIt != m_balls->end(); ++nestedIt) {
            Ball* ballB = *nestedIt;
            if (ballB == nullptr) continue;
            if (isColliding(ballA, ballB)) {
                // retrieve the changes in velocities for each ball and resolve collision
                QVector2D ballADeltaV,ballBDeltaV;
                std::tie(ballADeltaV, ballBDeltaV) = resolveCollision(ballA, ballB);

                // add screenshake, remove ball, and add children to table vector if breaking
                if (ballA->applyBreak(ballADeltaV, toBeAdded)) {
                    toBeRemoved.push_back(ballA);
                    incrementShake();
                    // nullify this ball
                    *it = nullptr;
                    break;
                }
                // add screenshake, remove ball, and add children to table vector if breaking
                if (ballB->applyBreak(ballBDeltaV, toBeAdded)) {
                    toBeRemoved.push_back(ballB);
                    incrementShake();
                    // nullify this ball
                    *nestedIt = nullptr;
                    continue;
                }
            }
        }
        // we marked this ball as deleted, so skip
        if (*it == nullptr) continue;

        // move ball due to speed
        ballA->translate(ballA->getVelocity() * dt);
        // apply friction
        ballA->changeVelocity(-ballA->getVelocity() * m_table->getFriction() * dt);
    }

    // clean up them trash-balls
    for (Ball* b : toBeRemoved) {
        delete b;
        // delete all balls marked with nullptr
        m_balls->erase(std::find(m_balls->begin(), m_balls->end(), nullptr));
    }
    for (Ball* b: toBeAdded) m_balls->push_back(b);

    updateShake(dt);
}

void Game::updateShake(double dt) {
    // <3 code lovingly taken from here: <3
    // https://gamedev.stackexchange.com/a/47565
    // slightly modified..

    // update the screen shake per time step
    m_shakeRadius *= (1-dt)*0.9;
    m_shakeAngle += (150 + rand()%60);
    m_screenshake = QVector2D(sin(m_shakeAngle)*m_shakeRadius, cos(m_shakeAngle)*m_shakeRadius);
}

std::vector<Pocket *> *Game::getPockets(){
    TableVisiter* visiter = new TableVisiter();
    visiter->visitTable(m_table);
    return visiter->getPockets();
}

QVector2D Game::resolveCollision(const Table* table, Ball* ball) {
    QVector2D bPos = ball->getPosition();

    QVector2D startingVel = ball->getVelocity();

    // resulting multiplicity of direction. If a component is set to -1, it
    // will flip the velocity's corresponding component
    QVector2D vChange(1,1);

    // ball is beyond left side of table's bounds
    if (bPos.x() - ball->getRadius() <= 0) {
        // flip velocity if wrong dir
        if (ball->getVelocity().x() <= 0) vChange.setX(-1);
    // ball is beyond right side of table's bounds
    } else if (bPos.x() + ball->getRadius() >= 0 + table->getWidth()) {
        // flip velocity if wrong dir
        if (ball->getVelocity().x() >= 0) vChange.setX(-1);
    }
    // ball is above top of the table's bounds
    if (bPos.y() - ball->getRadius() <= 0) {
        // flip iff we're travelling in the wrong dir
        if (ball->getVelocity().y() <= 0) vChange.setY(-1);
    // ball is beyond bottom of table's bounds
    } else if (bPos.y() + ball->getRadius() >= 0 + table->getHeight()) {
        // if we're moving down (we want to let the ball bounce up if its heading back)
        if (ball->getVelocity().y() >= 0) vChange.setY(-1);
    }

    ball->multiplyVelocity(vChange);

    // return the change in velocity
    return ball->getVelocity() - startingVel;
}

std::pair<QVector2D, QVector2D> Game::resolveCollision(Ball* ballA, Ball* ballB) {
    // SOURCE : ASSIGNMENT SPEC

    // if not colliding (distance is larger than radii)
    QVector2D collisionVector = ballB->getPosition() - ballA->getPosition();
    if (collisionVector.length() > ballA->getRadius() + ballB->getRadius()) {
       throw std::logic_error("attempting to resolve collision of balls that do not touch");
    }
    collisionVector.normalize();

    QVector2D ballAStartingVelocity = ballA->getVelocity();
    QVector2D ballBStartingVelocity = ballB->getVelocity();

    float mr = ballB->getMass() / ballA->getMass();
    double pa = QVector2D::dotProduct(collisionVector, ballA->getVelocity());
    double pb = QVector2D::dotProduct(collisionVector, ballB->getVelocity());

    if (pa <= 0 && pb >= 0) return std::make_pair(QVector2D(0,0), QVector2D(0,0));

    double a = -(mr + 1);
    double b = 2*(mr * pb + pa);
    double c = -((mr - 1)*pb*pb + 2*pa*pb);
    double disc = sqrt(b*b - 4*a*c);
    double root = (-b + disc)/(2*a);
    if (root - pb < 0.01) {
        root = (-b - disc)/(2*a);
    }

    ballA->changeVelocity(mr * (pb - root) * collisionVector);
    ballB->changeVelocity((root-pb) * collisionVector);

    // return the change in velocities for the two balls
    return std::make_pair(ballA->getVelocity() - ballAStartingVelocity, ballB->getVelocity() - ballBStartingVelocity);
}
