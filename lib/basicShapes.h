#pragma once
#include <array>
#include <glm/vec3.hpp>

template<typename T, glm::qualifier Q = glm::defaultp>
constexpr std::array<glm::vec<3, T, Q>, 8>
cuboidCorners(T lx, T ux, T ly, T uy, T lz, T uz)
{
	return {{
			{lx, uy, lz}, // LFB
			{ux, uy, lz}, // RFB
			{lx, uy, uz}, // LFT
			{ux, uy, uz}, // RFT
			{lx, ly, lz}, // LBB
			{ux, ly, lz}, // RBB
			{lx, ly, uz}, // LBT
			{ux, ly, uz}, // RBT
	}};
}
