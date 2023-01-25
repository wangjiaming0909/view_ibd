#pragma once

#include "page.h"
#include <cstdint>
namespace innodb {
namespace dd{
using DD_Table_id = uint64_t;

constexpr innodb::SpaceID dd_space_id = 0;

#define DD_SPACE "mysql"
#define DD_SPACE_ID 0
}
}
