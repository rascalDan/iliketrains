#pragma once

namespace OpenMesh {
	template<typename> class PolyMesh_ArrayKernelT;
}
struct ModelFactoryTraits;

using ModelFactoryMesh = OpenMesh::PolyMesh_ArrayKernelT<ModelFactoryTraits>;
