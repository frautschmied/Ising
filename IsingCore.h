#pragma once

// pybind
#include <pybind11/stl.h>

// print
#include <iostream>
#include <string>

// random
#include <random>
#include <functional>
#include <ctime>

// queue
#include <queue>
#include <set>

// model class
#include "MicroStates.h"

typedef std::pair<int, int> MEpair; // Magnetization-Energy Pair
typedef std::vector<MEpair> MEbuff; // Magnetization-Energy Buffer

typedef std::pair<int, int> XYpair; // Wolff Pair
typedef std::queue<XYpair> XYqueue; // Wolff Queue
typedef std::set<XYpair> XYset;     // Wolff Set