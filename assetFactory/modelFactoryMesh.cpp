#include "modelFactoryMesh.h"

ModelFactoryMesh::ModelFactoryMesh()
{
	add_property(smoothFaceProperty);
	add_property(materialFaceProperty);
	add_property(nameFaceProperty);
	add_property(nameAdjFaceProperty);
}

void
ModelFactoryMesh::configNamedFace(const std::string & name, OpenMesh::FaceHandle handle)
{
	property(nameFaceProperty, handle) = name;
	const auto halfEdges = fh_range(handle);
	for (const auto & he : halfEdges) {
		if (auto ofh = opposite_face_handle(he); ofh.is_valid()) {
			property(nameAdjFaceProperty, he) = property(nameFaceProperty, ofh);
		}
		if (auto oheh = opposite_halfedge_handle(he); oheh.is_valid()) {
			property(nameAdjFaceProperty, oheh) = name;
		}
	}
}
