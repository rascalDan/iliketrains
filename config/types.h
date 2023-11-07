#pragma once

#include "glad/gl.h"
#include <glm/geometric.hpp>

using Distance = float;
using Angle = float;

template<glm::length_t D> using Position = glm::vec<D, Distance>;
template<glm::length_t D> using Size = glm::vec<D, Distance>;
template<glm::length_t D> using Scale = glm::vec<D, float>;
template<glm::length_t D> using Direction = glm::vec<D, float>;
template<glm::length_t D> using Normal = Direction<D>;
template<glm::length_t D> using Rotation = glm::vec<D, Angle>;
template<glm::length_t Channels> using Colour = glm::vec<Channels, float>;

using Position2D = Position<2>;
using Position3D = Position<3>;
using Size2D = Size<2>;
using Size3D = Size<3>;
using Scale2D = Scale<2>;
using Scale3D = Scale<3>;
using Direction2D = Direction<2>;
using Direction3D = Direction<3>;
using Normal2D = Normal<2>;
using Normal3D = Normal<3>;
using Rotation2D = Rotation<2>;
using Rotation3D = Rotation<3>;
using TextureRelCoord = glm::vec<2, float>;
using TextureAbsCoord = glm::vec<2, GLsizei>;
using RGB = Colour<3>;
using RGBA = Colour<4>;
using ScreenRelCoord = glm::vec<2, float>;
using ScreenAbsCoord = glm::vec<2, uint16_t>;
