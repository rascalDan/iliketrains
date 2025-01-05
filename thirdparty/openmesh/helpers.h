#pragma once
#include <OpenMesh/Core/Mesh/BaseKernel.hh>

namespace OpenMesh::Helpers {
	template<typename Type, template<typename> typename PropertyT> struct Property : public PropertyT<Type> {
		template<typename... Params> explicit Property(OpenMesh::BaseKernel * kernel, Params &&... params)
		{
			kernel->add_property(*this, std::forward<Params>(params)...);
		}
	};
}
