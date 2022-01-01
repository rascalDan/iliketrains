#include "glArrays.h"
#include <type_traits>

// Base
static_assert(!std::is_default_constructible_v<glArraysBase<1>>);
static_assert(!std::is_copy_constructible_v<glArraysBase<1>>);
static_assert(!std::is_copy_assignable_v<glArraysBase<1>>);
static_assert(std::is_nothrow_move_constructible_v<glArraysBase<1>>);
static_assert(std::is_nothrow_move_assignable_v<glArraysBase<1>>);

// Specialisations (glBuffer is an example of the typedef)
static_assert(std::is_nothrow_default_constructible_v<glBuffer>);
static_assert(!std::is_trivially_default_constructible_v<glBuffer>);
static_assert(std::is_nothrow_destructible_v<glBuffer>);
static_assert(!std::is_trivially_destructible_v<glBuffer>);
static_assert(std::is_default_constructible_v<glBuffer>);
static_assert(!std::is_copy_constructible_v<glBuffer>);
static_assert(!std::is_copy_assignable_v<glBuffer>);
static_assert(std::is_nothrow_move_constructible_v<glBuffer>);
static_assert(std::is_nothrow_move_assignable_v<glBuffer>);
static_assert(sizeof(glBuffer) == sizeof(GLuint));
