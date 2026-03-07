#include <concepts>
#include <flat_map>
#include <glad/gl.h>
#include <memory>
#include <stream_support.h>
#include <vector>

namespace Detail {
	template<typename T> class glAllocator;
	template<typename C, typename T>
	concept IsGlBufferAllocated = requires(const C & container) {
		{ container.get_allocator() } -> std::same_as<glAllocator<T>>;
	};

	template<typename T> class glAllocator {
	public:
		// NOLINTBEGIN(readability-identifier-naming) - STL like
		using pointer = T *;
		using const_pointer = const T *;
		using value_type = T;

		// NOLINTEND(readability-identifier-naming)

		pointer
		allocate(size_t count)
		{
			constexpr static GLbitfield MAPPING_FLAGS
					= GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			constexpr static GLbitfield STORAGE_FLAGS = GL_DYNAMIC_STORAGE_BIT | MAPPING_FLAGS;
			GLuint name = 0;
			glCreateBuffers(1, &name);
			const auto size = static_cast<GLsizeiptr>(count * sizeof(T));
			glNamedBufferStorage(name, size, nullptr, STORAGE_FLAGS);
			const auto data = static_cast<pointer>(glMapNamedBufferRange(name, 0, size, MAPPING_FLAGS));
			if (!data) {
				glDeleteBuffers(1, &name);
				throw std::bad_alloc();
			}
			buffers->emplace(data, name);
			return data;
		}

		void
		deallocate(const_pointer ptr, size_t)
		{
			const auto itr = buffers->find(ptr);
			glUnmapNamedBuffer(itr->second);
			glDeleteBuffers(1, &itr->second);
			buffers->erase(itr);
		}

		[[nodiscard]] GLuint
		getNameFor(const_pointer ptr) const
		{
			const auto itr = buffers->find(ptr);
			if (itr != buffers->end()) {
				return itr->second;
			}
			return 0;
		}

		template<IsGlBufferAllocated<T> C>
		[[nodiscard]]
		GLuint
		getNameFor(const C & container) const
		{
			return getNameFor(container.data());
		}

		bool operator==(const glAllocator &) const = default;

	private:
		using BufferMap = std::flat_map<const_pointer, GLuint>;
		std::shared_ptr<BufferMap> buffers = std::make_shared<BufferMap>();
	};
}

template<typename T>
// NOLINTNEXTLINE(readability-identifier-naming) - OpenGL like
using glVector = std::vector<T, typename std::allocator_traits<Detail::glAllocator<T>>::allocator_type>;

template<typename C>
GLuint
operator*(const C & container)
	requires Detail::IsGlBufferAllocated<C, typename C::value_type>
{
	return container.get_allocator().getNameFor(container);
}

static_assert(Detail::IsGlBufferAllocated<glVector<int>, int>);
