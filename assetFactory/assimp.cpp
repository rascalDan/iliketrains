#include "assimp.h"
#include "assetFactory.h"
#include "collections.hpp"
#include "ptr.hpp"
#include "resource.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb/stb_image.h>

template<typename T>
glm::vec<3, T>
operator*(const aiVector3t<T> & v)
{
	return {v.x, v.y, v.z};
}

template<typename T>
glm::vec<2, T>
operator!(const aiVector3t<T> & v)
{
	return {v.x, v.y};
}

#define AIRANGE(parent, member) \
	std::span \
	{ \
		(parent)->m##member, (parent)->mNum##member \
	}

using ScemeCPtr = std::shared_ptr<const aiScene>;
class AssImpNode : public Shape {
public:
	AssImpNode(ScemeCPtr scene, const aiNode * node) : scene(std::move(scene)), node(node) { }

	ScemeCPtr scene;
	const aiNode * node;

	CreatedFaces
	createMesh(ModelFactoryMesh & mesh, float) const
	{
		CreatedFaces faces;
		addMesh(faces, mesh, node);
		return faces;
	}

	void
	addMesh(CreatedFaces & faces, ModelFactoryMesh & mesh, const aiNode * node) const
	{
		for (const auto & m : AIRANGE(node, Meshes)) {
			addMesh(faces, mesh, scene->mMeshes[m]);
		}
		for (const auto & n : AIRANGE(node, Children)) {
			addMesh(faces, mesh, n);
		}
	}

	void
	addMesh(CreatedFaces & faces, ModelFactoryMesh & mesh, const aiMesh * amesh) const
	{
		const auto vhs = AIRANGE(amesh, Vertices) * [&mesh](auto && v) {
			return mesh.add_vertex(*v);
		};
		const auto & m = *scene->mMaterials[amesh->mMaterialIndex];

		AssetFactory::TextureFragmentCoords tfc;
		if (auto mf = Persistence::ParseBase::getShared<AssetFactory>("assetFactory")) {
			aiString path;
			m.Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path);
			tfc = mf->getTextureCoords(path.C_Str());
		}

		for (const auto & f : AIRANGE(amesh, Faces)) {
			const auto fvhs = AIRANGE(&f, Indices) * [&vhs](auto && i) {
				return vhs[i];
			};
			const auto fh = faces.emplace(mesh.add_namedFace(amesh->mName.C_Str(), fvhs))->second;
			if (amesh->HasTextureCoords(0)) {
				for (auto idx = f.mIndices; const auto fheh : mesh.fh_range(fh)) {
					const auto ouv = !amesh->mTextureCoords[0][*idx++];
					const auto uv = glm::mix(tfc[0], tfc[2], ouv);
					mesh.set_texcoord2D(fheh, uv);
				}
			}
		}
	}
};

void
AssImp::postLoad()
{
	ScemeCPtr scene {
			aiImportFile(Resource::mapPath(path).c_str(), aiProcess_RemoveRedundantMaterials), &aiReleaseImport};
	if (!scene) {
		throw std::runtime_error("Failed to load asset library: " + path);
	}
	if (auto mf = Persistence::ParseBase::getShared<AssetFactory>("assetFactory")) {
		const auto root = AIRANGE(scene->mRootNode, Children);
		std::transform(
				root.begin(), root.end(), std::inserter(mf->shapes, mf->shapes.end()), [&scene](const aiNode * m) {
					return AssetFactory::Shapes::value_type {m->mName.C_Str(), std::make_shared<AssImpNode>(scene, m)};
				});
		const auto textures = AIRANGE(scene, Textures);
		std::transform(textures.begin(), textures.end(),
				std::inserter(mf->textureFragments, mf->textureFragments.end()), [](const aiTexture * t) {
					auto texture = std::make_shared<TextureFragment>();
					texture->id = texture->path = t->mFilename.C_Str();
					texture->image = std::make_unique<Image>(
							std::span {reinterpret_cast<unsigned char *>(t->pcData), t->mWidth}, STBI_rgb_alpha);
					return AssetFactory::TextureFragments::value_type {texture->id, texture};
				});
	}
}

bool
AssImp::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(path);
}
