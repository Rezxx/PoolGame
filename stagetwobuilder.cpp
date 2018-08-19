#include "stagetwobuilder.h"
#include "utils.h"
#include <iostream>
#include <QJsonArray>
#include <cassert>
#include <functional>
#include "ball.h"

QJsonObject StageTwoBuilder::addDefaultsToBall(QJsonObject bData, const char* defaultColour) {

    // default colour is white
    if (!bData.contains("colour")) {
        bData["colour"] = QString(defaultColour);
    }

    QString col = bData["colour"].toString(QString("Xx_420_noscope_invalidcolour_xX"));
    if (!isValidColour(col)) {
        std::cerr << "invalid ball colour supplied\n";
        bData["colour"] = "white";
    }

    // default mass is 1
    constexpr double defaultMass = 1.0;
    if (!bData.contains("mass")) bData["mass"] = defaultMass;
    if (bData["mass"].toDouble() == DOUBLEINF || bData["mass"].toDouble() <= 0) {
        std::cerr << "invalid ball mass\n";
        bData["mass"] = defaultMass;
    }

    // default strength is inf
    if (!bData.contains("strength")) bData["strength"] = DOUBLEINF;
    if (bData["strength"].toDouble(std::numeric_limits<double>::min()) == std::numeric_limits<double>::min()
                      || bData["strength"].toDouble() <= 0) {
        std::cerr << "invalid strength\n";
        bData["strength"] = DOUBLEINF;
    }

    // default radius is 10
    constexpr double defaultRadius = 10.0;
    if (!bData.contains("radius")) bData["radius"] = defaultRadius;
    double radius = bData["radius"].toDouble(DOUBLEINF);
    if (radius == DOUBLEINF || radius <= 0) {
        std::cerr << "invalid radius";
        bData["radius"] = defaultRadius;
    }

    // set default velocities
    {
        constexpr double defaultVelocity = 0.0;
        if (!bData.contains("velocity")) {
            bData["velocity"] = QJsonObject({{"x",defaultVelocity}, {"y",defaultVelocity}});
        } else {
            auto qjVel = bData.value("velocity").toObject();
            double velx = qjVel.value("x").toDouble(DOUBLEINF);
            double vely = qjVel.value("y").toDouble(DOUBLEINF);
            if (velx == DOUBLEINF) {
                std::cerr << "invalid xvelocity\n";
                velx = defaultVelocity;
            }
            if (vely == DOUBLEINF) {
                vely = defaultVelocity;
                std::cerr << "invalid yvelocity\n";
            }
            bData["velocity"] = QJsonObject({{"x",velx}, {"y",vely}});
        }
    }
    {
        constexpr double defaultPos = 0.0;
        if (!bData.contains("position")) {
            bData["position"] = QJsonObject({{"x",defaultPos}, {"y",defaultPos}});
        }
        auto qjPos = bData.value("position").toObject();
        double posx = qjPos.value("x").toDouble(DOUBLEINF);
        double posy = qjPos.value("y").toDouble(DOUBLEINF);
        if (posx == DOUBLEINF) {
            std::cerr << "invalid xvelocity\n";
            posx = defaultPos;
        }
        if (posy == DOUBLEINF) {
            std::cerr << "invalid yvelocity\n";
            posy = defaultPos;
        }
        bData["position"] = QJsonObject({{"x",posx}, {"y",posy}});
    }

    return bData;
}

bool StageTwoBuilder::isValidPocket(double xpos, double ypos, double radius, double tableWidth, double tableHeight) {
    if (xpos == DOUBLEINF || ypos == DOUBLEINF || radius == DOUBLEINF) return false;
    bool isWithinXBounds = xpos > -radius && xpos < tableWidth + radius;
    bool isWithinYBounds = ypos > -radius && ypos < tableHeight + radius;
    return isWithinXBounds && isWithinYBounds;
}

bool StageTwoBuilder::isValidColour(const QString& col) {
    // construct a QColour, if its not valid by Qt standards, return false
    QColor c(col);
    return c.isValid();
}

QJsonObject StageTwoBuilder::convertToValidTable(QJsonObject in) {
    // default table is 600x300
    auto defaultSize = QJsonObject({{"x",600.0},{"y",300.0}});

    // set default table size
    if (!in.contains("size")) in["size"] = defaultSize;
    double tableWidth = in.value("size").toObject()["x"].toDouble(DOUBLEINF);
    double tableHeight = in.value("size").toObject()["y"].toDouble(DOUBLEINF);
    // invalid value
    if (tableWidth == DOUBLEINF) {
        std::cerr << "invalid table width!\n";
        tableWidth = defaultSize.value("x").toDouble();
    }
    if (tableHeight == DOUBLEINF) {
        std::cerr << "invalid table height!\n";
        tableHeight = defaultSize.value("y").toDouble();
    }
    in["size"] = QJsonObject({{"x",tableWidth}, {"y",tableHeight}});

    // default colour of table is green
    if (!in.contains("colour")) in["colour"] = "green";
    // set the default to a colour that will probably never be valid, and test if valid
    if (!isValidColour(in["colour"].toString("Xx_420_noscope_invalidcolour_xX"))) {
        std::cerr << "invalid table colour\n";
        in["colour"] = "green";
    }

    // default friction is 0.01
    if (!in.contains("friction")) in["friction"] = 0.01;
    if (in["friction"].toDouble(DOUBLEINF) == DOUBLEINF) {
        std::cerr << "invalid friction supplied.\n";
        in["friction"] = 0.01;
    }

    return in;
}

void StageTwoBuilder::addBall(QJsonObject &ballData) {

    // if we haven't already started building
    if (m_buildingBalls == nullptr) {
        m_buildingBalls = new std::vector<Ball*>();
    }

    // clean & set the defaults for the top level ball
    QJsonObject cleanerBall = addDefaultsToBall(ballData);
    assert(!cleanerBall.isEmpty() && "somehow the balls hasn't been created properly?");

    Ball* b = m_factory->makeBall(cleanerBall);
    // ensure that the ball is within the table bounds
    if (!ballWithinTable(b, m_buildingTable)) {
        std::cerr << "ball placed off table, ignoring\n";
        delete b;
        return;
    }

    // generate the balls & add as children the nested balls, returning the generated ball, C++11 style
    std::function<Ball*(const QJsonObject&, const double&, const std::string&)> genBalls = [&](const QJsonObject& inV, const double& parentRadius, const std::string& parentColour) -> Ball* {
        // update defaults, but ignore ball if invalid
        QJsonObject cleanIn = convertAndCheckBall(inV, parentRadius, parentColour);
        if (cleanIn.isEmpty()) return nullptr;

        Ball* innerB = m_factory->makeBall(cleanIn);
        // leaf ball doesn't have children, i.e. non-composite
        if (!cleanIn.contains("balls")) return innerB;

        // generate and add children for each of the balls
        CompositeBall* compB = static_cast<CompositeBall*>(innerB);
        const double radius = cleanIn["radius"].toDouble();
        const std::string colour = cleanIn["colour"].toString().toStdString();

        for (const QJsonValueRef ov : cleanIn.value("balls").toArray()) {
            QJsonObject tmp = ov.toObject();
            // add as a child if the ball is   valid
            Ball* c = genBalls(tmp, radius, colour);
            if (c != nullptr) compB->addChild(c);
        }
        return innerB;
    };

    // add all of our children if they exist
    {
        const double radius = cleanerBall["radius"].toDouble();
        const std::string colour = cleanerBall["colour"].toString().toStdString();
        // recursively add composite balls
        CompositeBall* b2 = dynamic_cast<CompositeBall*>(b);
        if (b2 != nullptr) {
            for (const QJsonValueRef v : cleanerBall.value("balls").toArray()) {
                QJsonObject tmp = v.toObject();

                // generate and ensure child ball is valid
                Ball* c = genBalls(tmp, radius, colour);
                if (c != nullptr) {
                    b2->addChild(c);
                } else {
                    std::cerr << "invalid child ball, ignoring\n";
                }
            }
        }
    }

    m_buildingBalls->push_back(b);
}

void StageTwoBuilder::addTable(QJsonObject &tableDatax) {
    // ensure that we haven't already created another table
    if (m_buildingTable != nullptr) throw std::invalid_argument("table created twice");

    // make the json supplied compliant (doesn't check pockets)
    QJsonObject cookedTableData = convertToValidTable(tableDatax);
    StageTwoTable* t = static_cast<StageTwoTable*>(m_factory->makeTable(cookedTableData));

    // if we need to make pockets
    if (tableDatax.contains("pockets")) {
        for (const QJsonValueRef v : cookedTableData.value("pockets").toArray()) {
            // clean and check pockie
            QJsonObject pockieData = convertAndCheckPocket(v.toObject(), t->getWidth(), t->getHeight());
            // skip invalid pockie
            if (pockieData.isEmpty()) continue;

            t->addPocket(m_factory->makePocket(pockieData));
        }
    }

    m_buildingTable = static_cast<Table*>(t);
}

Game* StageTwoBuilder::getResult() {
    // no table?
    if (m_buildingTable == nullptr) {
        // add a default table
        std::cerr << "no table supplied...\n";
        auto defaultTable = QJsonObject();
        addTable(defaultTable);
    }
    // no balls?
    if (m_buildingBalls == nullptr) {
        // soft fail
        std::cerr << "warning! pool game without balls created...\n";
        QJsonObject pos({{"x", m_buildingTable->getWidth()/2}, {"y", m_buildingTable->getHeight()/2}});

        auto defaultBall = QJsonObject({{QString("position"),QJsonValue(pos)}});
        addBall(defaultBall);
    }

    // set the first ball to be cue ball
    // XXX: Patrick didn't follow the rules (first WHITE)
    Ball* c = m_buildingBalls->front();
    CueBall* cb = new CueBall(c);
    m_buildingBalls->front() = static_cast<Ball*>(cb);

    // just for fun, lets make a random ball have a trail
    size_t ind = rand()%m_buildingBalls->size();
    Ball* sparkleBall = m_buildingBalls->at(ind);
    m_buildingBalls->at(ind) =  new BallSparkleDecorator(sparkleBall);

    // and a random ball have bump effects
    ind = rand()%m_buildingBalls->size();
    Ball* bumpBall = m_buildingBalls->at(ind);
    m_buildingBalls->at(ind) = new BallSmashDecorator(bumpBall);

    Game* retGame = new Game(m_buildingBalls, m_buildingTable);
    // register all the mouse functions for the cueball
    retGame->addMouseFunctions(cb->getEvents());

    // need to reset for when we build next
    m_buildingBalls = nullptr;
    m_buildingTable = nullptr;

    return retGame;
}

bool StageTwoBuilder::ballWithinTable(const Ball *ball, const Table *table) {
    QVector2D bPos = ball->getPosition();
    // ball is beyond left side of table's bounds
    if (bPos.x() - ball->getRadius() <= 0) {
        return false;
    // ball is beyond right side of table's bounds
    } else if (bPos.x() + ball->getRadius() >= 0 + table->getWidth()) {
        return false;
    }

    // ball is above top of the table's bounds
    if (bPos.y() - ball->getRadius() <= 0) {
        return false;
    // ball is beyond bottom of table's bounds
    } else if (bPos.y() + ball->getRadius() >= 0 + table->getHeight()) {
        return false;
    }

    return true;
}

QJsonObject StageTwoBuilder::convertAndCheckBall(QJsonObject bData, const double parentRadius, const std::string &parentColour) {
    // update defaults (necessary to do first so that we can set default radii, and pos)
    bData = addDefaultsToBall(bData, parentColour.c_str());

    const double radius = bData["radius"].toDouble();
    QJsonObject pos = bData["position"].toObject();
    const double xpos = pos["x"].toDouble();
    const double ypos = pos["y"].toDouble();

    // return nothing if outside of parent ball
    if (!(parentRadius >= radius + xpos && parentRadius >= radius + ypos)) {
        return QJsonObject();
    }

    return bData;
}

QJsonObject StageTwoBuilder::convertAndCheckPocket(QJsonObject in, double tableWidth, double tableHeight) {
    // default radius = 15
    if (!in.contains("radius")) in["radius"] = 15.0;
    double radius = in.value("radius").toDouble(DOUBLEINF);
    // missing key
    if (!in.contains("position")) return QJsonObject();

    double xpos = in.value("position").toObject().value("x").toDouble(DOUBLEINF);
    double ypos = in.value("position").toObject().value("y").toDouble(DOUBLEINF);

    // return empty pocket, to indicate its not valid
    if (!isValidPocket(xpos, ypos, radius, tableWidth, tableHeight)) return QJsonObject();

    return in;
}
