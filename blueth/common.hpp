#pragma once

#if defined(__clang__) || defined(__GNUC__)
#define BLUETH_FORCE_INLINE [[gnu::always_inline]]
#elif
#define BLUETH_FORCE_INLINE
#endif
#define BLUETH_NODISCARD [[nodiscard]]
