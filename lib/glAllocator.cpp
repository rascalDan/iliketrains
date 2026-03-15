#include "glAllocator.h"

namespace Detail {
	std::pair<void *, GLuint>
	allocateBuffer(size_t count, size_t objSize)
	{
		constexpr static GLbitfield MAPPING_FLAGS
				= GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		constexpr static GLbitfield STORAGE_FLAGS = GL_DYNAMIC_STORAGE_BIT | MAPPING_FLAGS;
		GLuint name = 0;
		glCreateBuffers(1, &name);
		const auto size = static_cast<GLsizeiptr>(count * objSize);
		glNamedBufferStorage(name, size, nullptr, STORAGE_FLAGS);
		const auto data = (glMapNamedBufferRange(name, 0, size, MAPPING_FLAGS));
		if (!data) {
			glDeleteBuffers(1, &name);
			throw std::bad_alloc();
		}
		return {data, name};
	}

	void
	deallocateBuffer(GLuint name)
	{
		glUnmapNamedBuffer(name);
		glDeleteBuffers(1, &name);
	}
}
