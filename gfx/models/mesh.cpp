#include "mesh.h"

MeshBase::MeshBase(GLsizei m_numIndices, GLenum mode, const std::vector<RelativePosition3D> & positions) :
	m_numIndices {m_numIndices}, mode {mode}, dimensions {positions}
{
}

MeshBase::Dimensions::Dimensions(const std::span<const RelativePosition3D> positions) :
	Dimensions {positions, {extents(positions, 0), extents(positions, 1), extents(positions, 2)}}
{
}

MeshBase::Dimensions::Dimensions(
		const std::span<const RelativePosition3D> positions, const std::array<Extents1D, 3> & extents1ds) :
	minExtent(extents1ds[0].min, extents1ds[1].min, extents1ds[2].min),
	maxExtent(extents1ds[0].max, extents1ds[1].max, extents1ds[2].max), centre {(minExtent + maxExtent) / 2.0F},
	size {std::ranges::max(positions | std::views::transform([this](const auto & v) {
		return glm::distance(v, centre);
	}))}
{
}

MeshBase::Dimensions::Extents1D
MeshBase::Dimensions::extents(const std::span<const RelativePosition3D> positions, glm::length_t D)
{
	return std::ranges::minmax(positions | std::views::transform([D](const auto & v) {
		return v[D];
	}));
}

void
MeshBase::Draw() const
{
	glBindVertexArray(m_vertexArrayObject);

	glDrawElements(mode, m_numIndices, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
}

void
MeshBase::DrawInstanced(GLuint vao, GLsizei count, GLuint base) const
{
	glBindVertexArray(vao);

	glDrawElementsInstancedBaseInstance(mode, m_numIndices, GL_UNSIGNED_INT, nullptr, count, base);

	glBindVertexArray(0);
}
