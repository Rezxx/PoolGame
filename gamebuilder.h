#pragma once
#include "stageonefactory.h"
#include "stagetwofactory.h"
#include "game.h"

class GameBuilder {
protected:
    AbstractStageFactory* m_factory = nullptr;
    std::vector<Ball*>* m_buildingBalls = nullptr;
    Table* m_buildingTable = nullptr;
public:
    virtual ~GameBuilder();
    GameBuilder(AbstractStageFactory* factory) : m_factory(factory) {}
    /**
     * @brief addBall creates a ball to the current game being buil
     * @param ballData - json object that is simply an element of the array of balls provided in the config
     */
    virtual void addBall(QJsonObject& ballData) = 0;

    /**
     * @brief addTable creates a table to the current game being built
     * @param tableData - json object that contains all properties of the table
     */
    virtual void addTable(QJsonObject& tableData) = 0;

    /**
     * @brief getResult - retrieve the built object
     * @return
     */
    virtual Game* getResult();
};

class StageOneBuilder : public GameBuilder {
public:
    ~StageOneBuilder() {}
    StageOneBuilder() : GameBuilder(new StageOneFactory()) {}

    /**
     * @brief addBall creates a ball to the current game being built
     * @param ballData - json object that is simply an element of the array of balls provided in the config
     */
    void addBall(QJsonObject &ballData) override;

    /**
     * @brief addTable creates a table to the current game being built
     * @param tableData - json object that contains all properties of the table
     */
    void addTable(QJsonObject& tableData) override;
};

class GameDirector {
    GameBuilder* m_builder;
    const QJsonObject* m_conf;
public:
    ~GameDirector() { delete m_builder; /* ): json object is owned by other */ }
    GameDirector(QJsonObject* conf) : m_conf(conf) {}
    /**
     * @brief setBuilder - change which builder should be used for construction
     * @param newBuilder - the new builder
     */
    void setBuilder(GameBuilder* newBuilder) { m_builder = newBuilder; }
    /**
     * @brief createGame - retrieve the building that our owned builder created
     * @return - the newly created game
     */
    Game* createGame();
};
