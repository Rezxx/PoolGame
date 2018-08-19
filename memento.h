#ifndef MEMENTO_H
#define MEMENTO_H
#include "game.h"
/**
 * @brief The Memento class creates a copy of current game for restoring later
 */
class Memento
{
public:
    virtual ~Memento(){}

private:
    // only allows Originator to access the private member variables and functions
    friend class Originator;

    //constructed with the copy of the game
    Memento(Game* game){m_game = game->clone();}

    /**
     * @return the game that been saved
     */
    Game* getGame(){return m_game;}

private:
    Game* m_game;
};

#endif // MEMENTO_H
