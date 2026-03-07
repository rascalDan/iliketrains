#pragma once

#include "collections.h"
#include "glArrays.h"
#include "glBuffer.h"
#include "gl_traits.h"

namespace Impl {
	class VertexArrayConfigurator {
	public:
		template<typename M, typename T> struct MP {
			constexpr MP(M T::* ptr) : ptr {ptr} { }

			operator GLuint() const
			{
				return static_cast<GLuint>(reinterpret_cast<const char *>(&(static_cast<T *>(nullptr)->*ptr))
						- static_cast<const char *>(nullptr));
			}

			M T::* ptr;
			using ValueType = M;
		};

		template<typename M, typename T> MP(M T::*) -> MP<M, T>;

		explicit VertexArrayConfigurator(GLuint name) : name {name} { }

		VertexArrayConfigurator &
		addIndices(const glBuffer & buffer)
		{
			glVertexArrayElementBuffer(name, buffer);
			return *this;
		}

		VertexArrayConfigurator &
		addIndices(glBuffer & buffer, const SequentialCollection<GLuint> auto & indices)
		{
			buffer.storage(indices, 0);
			return addIndices(buffer);
		}

		// Customisation point
		template<typename VertexT> VertexArrayConfigurator & addAttribsFor(GLuint divisor, const glBuffer & buffer);

		template<typename VertexT, MP... Attribs>
		VertexArrayConfigurator &
		addAttribs(const GLuint divisor)
		{
			configureAttribs<VertexT, Attribs...>(divisor);
			return *this;
		}

		template<typename VertexT, MP... Attribs>
		VertexArrayConfigurator &
		addAttribs(const GLuint divisor, const glBuffer & buffer)
		{
			glVertexArrayVertexBuffer(name, binding, buffer, 0, sizeof(VertexT));
			return addAttribs<VertexT, Attribs...>(divisor);
		}

		template<typename VertexT, MP... Attribs>
		VertexArrayConfigurator &
		addAttribs(const GLuint divisor, glBuffer & buffer, const SequentialCollection<VertexT> auto & data)
		{
			buffer.storage(data, 0);
			return addAttribs<VertexT, Attribs...>(divisor, buffer);
		}

	private:
		void
		setPointerMeta(const GLuint usedAttribs)
		{
			while (attrib < usedAttribs) {
				glEnableVertexArrayAttrib(name, attrib);
				glVertexArrayAttribBinding(name, attrib++, binding);
			}
		}

		template<typename T>
		void
		setPointer(const GLuint offset)
		{
			setPointerMeta(attrib + gl_traits<T>::vertexArrayAttribFormat(name, attrib, offset));
		}

		template<MP Attrib>
		void
		setPointer()
		{
			setPointer<typename decltype(Attrib)::ValueType>(Attrib);
		}

		template<typename VertexT, MP... Attribs>
		void
		configureAttribs(const GLuint divisor)
		{
			if constexpr (sizeof...(Attribs) == 0) {
				setPointer<VertexT>(0);
			}
			else {
				((setPointer<Attribs>()), ...);
			}
			glVertexArrayBindingDivisor(name, binding++, divisor);
		}

		GLuint name;
		GLuint binding = 0;
		GLuint attrib = 0;
	};

	// NOLINTNEXTLINE(readability-identifier-naming)
	struct glVertexArray : Detail::glNamed {
		VertexArrayConfigurator
		configure()
		{
			return VertexArrayConfigurator {name};
		}

		template<typename GlAllocated>
		void
		useBuffer(GLuint binding, const GlAllocated & buffer) const
			requires requires {
				{ buffer.bufferName() } -> std::same_as<GLuint>;
			}
		{
			using T = typename GlAllocated::value_type;
			glVertexArrayVertexBuffer(name, binding, buffer.bufferName(), 0, sizeof(T));
		}
	};
}

// NOLINTBEGIN(readability-identifier-naming)
template<size_t N>
using glVertexArrays = glManagedArray<Impl::glVertexArray, N, &glCreateVertexArrays, &glDeleteVertexArrays>;
using glVertexArray = glManagedSingle<Impl::glVertexArray, &glCreateVertexArrays, &glDeleteVertexArrays>;
// NOLINTEND(readability-identifier-naming)
