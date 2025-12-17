#ifndef PCHDEF_H
#define PCHDEF_H

// std libs
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <deque>
#include <algorithm>
#include <cstring>

// third-party
#include <fmt/core.h>
#include <nlohmann/json.hpp>

// stable headers
#include "Misc/Define.h"
#include "Misc/WowGuid.h"
#include "Misc/Logger.h"
#include "Misc/Utilities.h"
#include "Misc/Exceptions.h"

#include "Reader/BitReader.h"

// structures
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"

#endif