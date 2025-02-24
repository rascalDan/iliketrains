#include "modelFactoryMesh.h"

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
