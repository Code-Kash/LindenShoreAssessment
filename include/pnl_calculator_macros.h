#pragma once

namespace pnl::macros
{
    #define RULE_OF_FIVE_COPYABLE(ClassName) \
        ClassName(const ClassName&) = default; \
        ClassName& operator=(const ClassName&) = default; \
        ClassName(ClassName&&) = default; \
        ClassName& operator=(ClassName&&) = default; \
        ~ClassName() = default;

    #define RULE_OF_FIVE_TRIVIALLY_COPYABLE(ClassName) \
        ClassName(const ClassName&) = default; \
        ClassName& operator=(const ClassName&) = default; \
        ClassName(ClassName&&) = default; \
        ClassName& operator=(ClassName&&) = default; \
        virtual ~ClassName() = default;

    #define RULE_OF_FIVE_MOVABLE(ClassName) \
        ClassName(const ClassName&) = delete; \
        ClassName& operator=(const ClassName&) = delete; \
        ClassName(ClassName&&) = default; \
        ClassName& operator=(ClassName&&) = default; \
        ~ClassName() = default;

    #define RULE_OF_FIVE_NONMOVABLE(ClassName) \
        ClassName(const ClassName&) = delete; \
        ClassName& operator=(const ClassName&) = delete; \
        ClassName(ClassName&&) = delete; \
        ClassName& operator=(ClassName&&) = delete; \
        ~ClassName() = default;

    #define CACHE_LINE_ALIGNED alignas(64)
    #define LIKELY [[likely]]
    #define UNLIKELY [[unlikely]]
    #define FORCE_INLINE [[gnu::always_inline]] inline
    #define NO_INLINE [[gnu::noinline]]
}