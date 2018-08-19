#include "dialog.h"
#include "ui_dialog.h"
#include <QPainter>
#include <QTimer>
#include <iostream>
#include <QMouseEvent>
#include "utils.h"

Dialog::Dialog(Game *game, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    m_game(game)
{
    if(m_game->isStageThree()){
        //initialise the originator when the game is in stageThree
        m_orig = new Originator(game);
    }else{
        m_orig = NULL;
    }
    ui->setupUi(this);

    // for animating (i.e. movement, collision) every animFrameMS
    aTimer = new QTimer(this);
    connect(aTimer, SIGNAL(timeout()), this, SLOT(nextAnim()));
    aTimer->start(animFrameMS);

    // for drawing every drawFrameMS milliseconds
    dTimer = new QTimer(this);
    connect(dTimer, SIGNAL(timeout()), this, SLOT(tryRender()));
    dTimer->start(drawFrameMS);

    // set the window size to be at least the table size
    this->resize(game->getMinimumWidth(), game->getMinimumHeight());
}


void Dialog::restore(){
    if(m_memos.size() > 0){
        delete m_game;
        Memento* memo = m_memos.top();//get the most recently saved game
        m_memos.pop();//remove that game from the stack
        m_orig->restore(memo);
        m_game = m_orig->getGame();
    }
}

Dialog::~Dialog()
{
    delete aTimer;
    delete dTimer;
    delete m_game;
    delete ui;
}

void Dialog::tryRender() {
    this->update();
}

void Dialog::nextAnim() {
    m_game->animate(1.0/(double)animFrameMS);
    if(m_game->toSave()){
        m_memos.push(m_orig->createMomento());
        m_game->notSave();
    }
}

void Dialog::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    m_game->render(painter);
}

void Dialog::mousePressEvent(QMouseEvent* event) {
    evalAllEventsOfTypeSpecified(MouseEventable::EVENTS::MouseClickFn, event);
}

void Dialog::mouseReleaseEvent(QMouseEvent* event) {
    evalAllEventsOfTypeSpecified(MouseEventable::EVENTS::MouseRelFn, event);
}
void Dialog::mouseMoveEvent(QMouseEvent* event) {
    evalAllEventsOfTypeSpecified(MouseEventable::EVENTS::MouseMoveFn, event);
}

void Dialog::keyPressEvent(QKeyEvent * event){
    //only works in stage3
    if(m_game->isStageThree()){
        if(event->key() == Qt::Key_R){
            restore(); // restore the game when pressed R
        }else if(event->key() == Qt::Key_S){
            m_game->switchMode(); //swtich the strategy when pressed S
        }else if(event->key() == Qt::Key_A){
            m_game->addRandomBall(); //add ball when pressed A
        }
    }
}


void Dialog::evalAllEventsOfTypeSpecified(MouseEventable::EVENTS t, QMouseEvent *event) {
    // handle all the clicky events, and remove them if they've xPIRED
    MouseEventable::EventQueue& Qu = m_game->getEventFns();
    for (ssize_t i = Qu.size()-1; i >= 0; i--) {
        if (auto spt = (Qu.at(i)).lock()) {
            if (spt->second == t) {
                spt->first(event);
            }
        } else {
            // remove this element from our vector
            Qu.erase(Qu.begin() + i);
        }
    }
}
