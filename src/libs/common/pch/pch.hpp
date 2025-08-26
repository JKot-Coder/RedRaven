#pragma once

// IWYU pragma: begin_exports
#include "common/Config.hpp"
#include "common/NonCopyableMovable.hpp"
#include "common/debug/Logger.hpp"

#ifdef __cplusplus
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <fmt/core.h>
#include <fmt/printf.h>
#include <functional>
#include <memory>
#include <vector>
#include <string_view>

#include <EASTL/array.h>
#include <EASTL/memory.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/string_view.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/weak_ptr.h>

#define EASTL_FRIEND_MAKE_SHARED template <typename, typename> friend class eastl::ref_count_sp_t_inst

using namespace RR::Common::Debug;

#include "math/Base.hpp"
// IWYU pragma: end_exports

#else
#include <cstddef>
#include <stdint.h>
#endif
