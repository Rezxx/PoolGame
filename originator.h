#ifndef ORIGINATOR_H
#define ORIGINATOR_H
#include "memento.h"
/**
 * @brief The Originator class in chrage of creating memento and restore the game from memento
 */
class Originator
{
public:
    Originator(Game* game):m_game(game){}

    /**
     * @return the game after the restoration
     */
    Game* getGame(){return m_game;}

    /**
     * @return a memento with current game
     */
    Memento* createMomento(){return new Memento(m_game);}

    /**
     * @brief restores the game with given memento
     * @param memento contains to be restored game
     */
    void restore(Memento* memento){m_game = memento->getGame();}

private:
    Game* m_game;
};

#endif // ORIGINATOR_H
