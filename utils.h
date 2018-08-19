#pragma once

#include <QVector2D>
#include <limits>
#include <functional>
#include <QMouseEvent>
#include <memory>
#include <type_traits>
#include <iostream>

/* filename of the config file */
constexpr char config_path[] = "../../../../Poolgame/config.json";

constexpr int animFrameMS = 10;
constexpr int drawFrameMS = 10;

constexpr double DOUBLEINF = std::numeric_limits<double>::max();
