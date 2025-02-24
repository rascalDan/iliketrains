#pragma once
#include <OpenMesh/Core/Mesh/BaseKernel.hh>
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <ranges>

namespace OpenMesh {
	template<typename Iter, typename CenterEntityHandle>
	using IteratorFunction = Iter (OpenMesh::PolyConnectivity::*)(CenterEntityHandle) const;

	template<typename Iter, typename CenterEntityHandle, IteratorFunction<Iter, CenterEntityHandle> BeginFunc,
			IteratorFunction<Iter, CenterEntityHandle> EndFunc, typename Adaptor>
	auto
	operator|(const OpenMesh::PolyConnectivity::CirculatorRange<OpenMesh::PolyConnectivity, Iter, CenterEntityHandle,
					  BeginFunc, EndFunc> & range,
			Adaptor && adaptor)
	{
		return std::views::iota(range.begin(), range.end()) | std::forward<Adaptor>(adaptor);
	}

	namespace Helpers {
		template<typename Type, template<typename> typename PropertyT> struct Property : public PropertyT<Type> {
			template<typename... Params> explicit Property(OpenMesh::BaseKernel * kernel, Params &&... params)
			{
				kernel->add_property(*this, std::forward<Params>(params)...);
			}
		};
	}
}
