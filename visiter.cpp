#include "visiter.h"

std::vector<Pocket *> *TableVisiter::visitTable(Table *table)
{
    m_pockets = table->accept(this);
}
