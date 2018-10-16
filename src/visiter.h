#pragma once

#include "pocket.h"
#include "table.h"

class Table;
/**
 * @brief The Visiter class defines interface for visiter to visit other classes
 */
class Visiter
{
public:
    Visiter(){}
    ~Visiter(){}
    virtual std::vector<Pocket*>* visitTable(Table* table) = 0;
};

/**
 * @brief The TableVisiter class can visit table and retrieve the balls from it
 */
class TableVisiter : public Visiter{
public:
    TableVisiter(): m_pockets(NULL){}

    /**
     * @brief visitTable - visit the table and bring back the pockets
     * @param table - target table
     */
    std::vector<Pocket*>* visitTable(Table* table) override;

    /**
     * @return retrieved pockets
     */
    std::vector<Pocket*>* getPockets(){return m_pockets;}
private:
    std::vector<Pocket*>* m_pockets;
};
