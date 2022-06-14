

#include "format_check.hpp"

static_assert(hi::format_count("") == 0);
static_assert(hi::format_count("foo") == 0);
static_assert(hi::format_count("{{}}") == 0);

static_assert(hi::format_count("{}") == 1);
static_assert(hi::format_count("{{{}") == 1);
static_assert(hi::format_count("{}}}") == 1);
static_assert(hi::format_count("{{}}{}") == 1);
static_assert(hi::format_count("{{{}}}") == 1);
static_assert(hi::format_count("{{{{{}}}}}") == 1);
static_assert(hi::format_count("foo{}") == 1);
static_assert(hi::format_count("foo{}") == 1);
static_assert(hi::format_count("{}bar") == 1);
static_assert(hi::format_count("foo{}bar") == 1);

static_assert(hi::format_count("{}{}") == 2);
static_assert(hi::format_count("{} {}") == 2);

static_assert(hi::format_count("{1:} {2:}") == 2);
static_assert(hi::format_count("{s} {}") == 2);

// Invalid open brace
static_assert(hi::format_count("{1:{}") == -1);

// Invalid close brace
static_assert(hi::format_count("foo }") == -2);

// Missing close brace
static_assert(hi::format_count("{:1 foo") == -3);

