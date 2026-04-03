#pragma once

#include "collections.h"
#include "glArrays.h"
#include "glBuffer.h"
#include "gl_traits.h"
#include "util.h"

namespace Impl {
	class VertexArrayConfigurator {
	public:
		template<typename M, typename T> struct MP {
			constexpr MP(M T::* ptr) : ptr {ptr} { }

			constexpr
			operator GLuint() const
			{
				constexpr static char dummy {};
				return static_cast<GLuint>(reinterpret_cast<const char *>(&(reinterpret_cast<const T *>(&dummy)->*ptr))
						- reinterpret_cast<const char *>(&dummy));
			}

			M T::* ptr;
		};

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
		template<typename VertexT> VertexArrayConfigurator & addAttribsFor(GLuint divisor);

		template<typename VertexT>
		VertexArrayConfigurator &
		addAttribsFor(GLuint divisor, const glBuffer & buffer)
		{
			glVertexArrayVertexBuffer(name, binding, buffer, 0, sizeof(VertexT));
			return addAttribsFor<VertexT>(divisor);
		}

		template<typename VertexT, auto... Attribs>
		VertexArrayConfigurator &
		addAttribs(const GLuint divisor)
		{
			configureAttribs<VertexT, Attribs...>(divisor);
			return *this;
		}

		template<typename VertexT, auto... Attribs>
		VertexArrayConfigurator &
		addAttribs(const GLuint divisor, const glBuffer & buffer)
		{
			glVertexArrayVertexBuffer(name, binding, buffer, 0, sizeof(VertexT));
			return addAttribs<VertexT, Attribs...>(divisor);
		}

		template<typename VertexT, auto... Attribs>
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

		template<typename VertexT, auto Attrib>
		void
		setPointer()
		{
			using Mbr = MemberValueType<Attrib>;
			setPointer<Mbr>(MP<Mbr, VertexT> {Attrib});
		}

		template<typename VertexT, auto... Attribs>
		void
		configureAttribs(const GLuint divisor)
		{
			if constexpr (sizeof...(Attribs) == 0) {
				setPointer<VertexT>(0);
			}
			else {
				((setPointer<VertexT, Attribs>()), ...);
			}
			glVertexArrayBindingDivisor(name, binding++, divisor);
		}

		GLuint name;
		GLuint binding = 0;
		GLuint attrib = 0;
	};

	struct glVertexArray : Detail::glNamed {
		VertexArrayConfigurator
		configure()
		{
			return VertexArrayConfigurator {name};
		}

		template<typename glAllocated>
		void
		useBuffer(GLuint binding, const glAllocated & buffer, GLsizei offset = 0) const
			requires requires {
				{ buffer.bufferName() } -> std::same_as<GLuint>;
			}
		{
			using T = typename glAllocated::value_type;
			useBuffer(binding, buffer.bufferName(), sizeof(T), offset);
		}

		template<typename V>
		void
		useBuffer(GLuint binding, GLuint bufferName, auto V::* mbr) const
		{
			useBuffer(binding, bufferName, sizeof(V), VertexArrayConfigurator::MP {mbr});
		}

		template<typename V>
		void
		useBuffer(GLuint binding, GLuint bufferName, GLintptr offset = 0) const
		{
			useBuffer(binding, bufferName, sizeof(V), offset);
		}

		void
		useBuffer(GLuint binding, GLuint bufferName, GLsizei stride, GLintptr offset = 0) const
		{
			glVertexArrayVertexBuffer(name, binding, bufferName, offset, stride);
		}
	};
}

template<size_t N>
using glVertexArrays = glManagedArray<Impl::glVertexArray, N, &glCreateVertexArrays, &glDeleteVertexArrays>;
using glVertexArray = glManagedSingle<Impl::glVertexArray, &glCreateVertexArrays, &glDeleteVertexArrays>;
