#pragma once

//...Namespace makes much more sense for this. Although I probably will need to
// implement virtual functions later. Just not now.
#include "types.h"
#include <vector>

namespace ActOS {
std::vector<ProcessInfoT> GetTargets();

}
