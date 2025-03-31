#pragma once

#include "glad/gl.h"
#include <glm/geometric.hpp>

using Distance = float; // deprecate
using RelativeDistance = float;
using GlobalDistance = int32_t;
using CalcDistance = int64_t;
using Angle = float;

template<glm::length_t D> using Position = glm::vec<D, Distance>; // deprecate
template<glm::length_t D> using RelativePosition = glm::vec<D, RelativeDistance>;
template<glm::length_t D> using GlobalPosition = glm::vec<D, GlobalDistance>;
template<glm::length_t D> using CalcPosition = glm::vec<D, CalcDistance>;
template<glm::length_t D> using Size = glm::vec<D, Distance>;
template<glm::length_t D> using Scale = glm::vec<D, float>;
template<glm::length_t D> using Direction = glm::vec<D, float>;
template<glm::length_t D> using Normal = Direction<D>;
template<glm::length_t D> using Rotation = glm::vec<D, Angle>;
template<glm::length_t Channels> using Colour = glm::vec<Channels, float>;

using Position3D = Position<3>; // deprecate
using BaryPosition = glm::vec<2, float>;
using RelativePosition2D = RelativePosition<2>;
using RelativePosition3D = RelativePosition<3>;
using RelativePosition4D = RelativePosition<4>;
using GlobalPosition2D = GlobalPosition<2>;
using GlobalPosition3D = GlobalPosition<3>;
using GlobalPosition4D = GlobalPosition<4>;
using CalcPosition2D = CalcPosition<2>;
using CalcPosition3D = CalcPosition<3>;
using CalcPosition4D = CalcPosition<4>;
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
using ImageDimensions = glm::vec<2, GLsizei>;
using TextureDimensions = glm::vec<3, GLsizei>;
using TextureRelRegion = glm::vec<4, float>;
using TextureAbsCoord = glm::vec<2, GLsizei>;
using TextureAbsRegion = glm::vec<4, GLsizei>;
using RGB = Colour<3>;
using RGBA = Colour<4>;
using ScreenRelCoord = glm::vec<2, float>;
using ScreenAbsCoord = glm::vec<2, uint16_t>;
using ViewPort = glm::vec<4, GLsizei>;
