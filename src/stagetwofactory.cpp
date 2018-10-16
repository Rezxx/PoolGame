#include "stagetwofactory.h"
#include <iostream>

Pocket* StageTwoFactory::makePocket(const QJsonObject &pocketData) {
    double radius = pocketData.value("radius").toDouble();
    QJsonObject pPos = pocketData.value("position").toObject();
    double xpos = pPos.value("x").toDouble();
    double ypos = pPos.value("y").toDouble();
    return new Pocket(radius, QVector2D(xpos, ypos));
}

Ball* StageTwoFactory::makeBall(const QJsonObject& ballData) {
    // construct a ball from its values

    QString col = ballData.value("colour").toString();

    // extract pos into two doubles
    QJsonObject tPos = ballData.value("position").toObject();
    double xpos = tPos.value("x").toDouble();
    double ypos = tPos.value("y").toDouble();

    // extract velocity into two doubles
    QJsonObject tVel = ballData.value("velocity").toObject();
    double xvel = tVel.value("x").toDouble();
    double yvel = tVel.value("y").toDouble();

    double mass = ballData.value("mass").toDouble();
    double radius = ballData.value("radius").toDouble();

    double strength = ballData.value("strength").toDouble();

    return new CompositeBall(col, QVector2D(xpos, ypos), QVector2D(xvel, yvel), mass, radius, strength);
}

Table* StageTwoFactory::makeTable(const QJsonObject& tableData) {

    // create a stage one table based on the fed in json data
    QString colour = tableData.value("colour").toString();

    // extract width and height from json vals
    QJsonObject tSize = tableData.value("size").toObject();
    double width = tSize.value("x").toDouble();
    double height = tSize.value("y").toDouble();

    double friction = tableData.value("friction").toDouble();

    // pockets are not added in yet.
    return new StageTwoTable(width, height, QColor(colour), friction);
}
