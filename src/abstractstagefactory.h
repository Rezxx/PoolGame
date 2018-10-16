#pragma once

#include "ball.h"
#include "table.h"
#include "pocket.h"

class AbstractStageFactory {
public:
    virtual ~AbstractStageFactory(){}
    /**
     * @brief makeBall - construct a ball based on the json provided
     * @param ballData - json that conforms to the spec
     * @return our newly created ball
     */
    virtual Ball* makeBall(const QJsonObject& ballData) = 0;
    /**
     * @brief makeTable - construct a table based on json provided
     * @param tableData - json that conforms to the spec
     * @return our newly created table
     */
    virtual Table* makeTable(const QJsonObject& tableData) = 0;

    /**
     * @brief makePocket - construct a pocket based on json
     * @param pocketData - the json
     * @return newly created pocket
     */
    virtual Pocket* makePocket(const QJsonObject& pocketData) = 0;
};
