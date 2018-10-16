#pragma once

#include "gamebuilder.h"
#include "utils.h"
#include <iostream>

class StageTwoBuilder : public GameBuilder {
public:
    ~StageTwoBuilder() {}
    StageTwoBuilder(): GameBuilder(new StageTwoFactory()) {}

    void addBall(QJsonObject &ballData) override;
    void addTable(QJsonObject &tableData) override;

    virtual Game* getResult() override;

private:
    // helper functions to ensure json is valid & well constructed

    /* return whether the pocket is valid */
    static bool isValidPocket(double xpos, double ypos, double radius, double tableWidth, double tableHeight);
    /* whether this colour can be used to create a qt colour */
    static bool isValidColour(const QString& col);

    /**
     * @brief convertAndCheckBall - set defaults in the json, and check whether the ball should be created
     * @param bData - the json to check and read data from
     * @param parentRadius - the parent's radius for this ball (check whether within the ball radii)
     * @param parentColour - the parent's colour, we set the ball's colour to this if not specified in bData
     * @return the resulting valid (or empty if invalid) bal data
     */
    static QJsonObject convertAndCheckBall(QJsonObject bData, const double parentRadius, const std::string& parentColour);

    // just update to defaults if missing values (non-recursive)
    /**
     * @brief addDefaultsToBall - just set the defaults for the ball (non-recursive) if they are not valid, or don't exist
     * @param bData - the starting ball data
     * @param defaultColour - the colour to set if not exists, or null
     * @return
     */
    static QJsonObject addDefaultsToBall(QJsonObject bData, const char* defaultColour = "white");

    /**
     * @brief ballWithinTable - return whether the ball is within the table's bounds
     * @param ball - the ball to check
     * @param table - the table to check
     * @return true if ball within bounds, false otherwise
     */
    static bool ballWithinTable(const Ball* ball, const Table* table);

    /**
     * @brief convertToValidTable set defaults for the tables, excluding the pockets
     * @param in - the json object that currently holds the settings for the tables
     * @return the resulting valid table object
     */
    static QJsonObject convertToValidTable(QJsonObject in);

    /**
     * @brief convertAndCheckPocket - convert the pocket to a valid one if possible (add defaults), or return an empty json if irrecoverably invalid
     * @param in - the pocket data to check
     * @param tableWidth - the width of the table that contains this pocket
     * @param tableHeight - the height of the table that contains this pocket
     * @return empty jsonobj if invalid, otherwise corrected pocket
     */
    static QJsonObject convertAndCheckPocket(QJsonObject in, double tableWidth, double tableHeight);
};
