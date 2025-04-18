#pragma once
#include <OpenMesh/Core/Mesh/BaseKernel.hh>
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <ranges>

namespace OpenMesh {
	template<typename Iter, typename... IterParams>
	using IteratorFunction = Iter (OpenMesh::PolyConnectivity::*)(IterParams...) const;

#if OM_GET_VER < 8
	template<typename Iter, typename CenterEntityHandle, IteratorFunction<Iter, CenterEntityHandle> BeginFunc,
			IteratorFunction<Iter, CenterEntityHandle> EndFunc, typename Adaptor>
	auto
	operator|(const OpenMesh::PolyConnectivity::CirculatorRange<OpenMesh::PolyConnectivity, Iter, CenterEntityHandle,
					  BeginFunc, EndFunc> & range,
			Adaptor && adaptor)
	{
		return std::views::iota(range.begin(), range.end()) | std::forward<Adaptor>(adaptor);
	}

	template<typename Iter, IteratorFunction<Iter> BeginFunc, IteratorFunction<Iter> EndFunc, typename Adaptor>
	auto
	operator|(
			const OpenMesh::PolyConnectivity::EntityRange<const OpenMesh::PolyConnectivity, Iter, BeginFunc, EndFunc> &
					range,
			Adaptor && adaptor)
	{
		return std::views::iota(range.begin(), range.end()) | std::forward<Adaptor>(adaptor);
	}
#else
	template<typename Iter, typename CenterEntityHandle, typename ToEntityHandle,
			IteratorFunction<Iter, CenterEntityHandle> BeginFunc, IteratorFunction<Iter, CenterEntityHandle> EndFunc,
			typename Adaptor>
	auto
	operator|(const CirculatorRange<CirculatorRangeTraitT<OpenMesh::PolyConnectivity, Iter, CenterEntityHandle,
					  ToEntityHandle, BeginFunc, EndFunc>> & range,
			Adaptor && adaptor)
	{
		return std::views::iota(range.begin(), range.end()) | std::forward<Adaptor>(adaptor);
	}

	template<typename Iter, IteratorFunction<Iter> BeginFunc, IteratorFunction<Iter> EndFunc, typename Adaptor>
	auto
	operator|(const EntityRange<RangeTraitT<const OpenMesh::PolyConnectivity, Iter, BeginFunc, EndFunc>> & range,
			Adaptor && adaptor)
	{
		return std::views::iota(range.begin(), range.end()) | std::forward<Adaptor>(adaptor);
	}

#endif

	namespace Helpers {
		template<typename Type, template<typename> typename PropertyT> struct Property : public PropertyT<Type> {
			template<typename... Params> explicit Property(OpenMesh::BaseKernel * kernel, Params &&... params)
			{
				kernel->add_property(*this, std::forward<Params>(params)...);
			}
		};
	}
}
