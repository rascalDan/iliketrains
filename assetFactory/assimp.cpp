#include "assimp.h"
#include "assetFactory.h"
#include "collections.hpp"
#include "ptr.hpp"
#include "resource.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <future>
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

using SceneCPtr = std::shared_ptr<const aiScene>;
class AssImpNode : public Shape {
public:
	AssImpNode(SceneCPtr scene, const aiNode * node) : scene(std::move(scene)), node(node) { }

	SceneCPtr scene;
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
		mesh.normalsProvidedProperty = amesh->HasNormals();
		const auto vhs = AIRANGE(amesh, Vertices) *
				[&mesh, normals = amesh->HasNormals() ? amesh->mNormals : nullptr](auto && v) mutable {
					const auto vh = mesh.add_vertex(*v);
					if (normals) {
						mesh.set_normal(vh, **(normals++));
					}
					return vh;
				};
		const auto & m = scene->mMaterials[amesh->mMaterialIndex]->GetName();

		GLuint material {};
		if (auto mf = Persistence::ParseBase::getShared<AssetFactory>("assetFactory")) {
			material = mf->getMaterialIndex(m.C_Str());
		}

		for (const auto & f : AIRANGE(amesh, Faces)) {
			const auto fvhs = AIRANGE(&f, Indices) * [&vhs](auto && i) {
				return vhs[i];
			};
			const auto fh = faces.emplace(mesh.add_namedFace(amesh->mName.C_Str(), fvhs))->second;
			if (amesh->HasTextureCoords(0)) {
				for (auto idx = f.mIndices; const auto fheh : mesh.fh_range(fh)) {
					const auto ouv = !amesh->mTextureCoords[0][*idx++];
					mesh.set_texcoord2D(fheh, ouv);
					mesh.property(mesh.materialFaceProperty, fh) = material;
				}
			}
		}
	}
};

static_assert(TextureOptions::MapMode::Repeat == static_cast<TextureOptions::MapMode>(aiTextureMapMode_Wrap));
static_assert(TextureOptions::MapMode::Clamp == static_cast<TextureOptions::MapMode>(aiTextureMapMode_Clamp));
static_assert(TextureOptions::MapMode::Decal == static_cast<TextureOptions::MapMode>(aiTextureMapMode_Decal));
static_assert(TextureOptions::MapMode::Mirror == static_cast<TextureOptions::MapMode>(aiTextureMapMode_Mirror));

void
AssImp::postLoad()
{
	SceneCPtr scene {
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
		const auto materials = AIRANGE(scene, Materials);
		std::transform(materials.begin(), materials.end(),
				std::inserter(mf->textureFragments, mf->textureFragments.end()), [&scene](const aiMaterial * m) {
					aiString path;
					m->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path);
					auto texture = std::make_shared<TextureFragment>();
					texture->id = m->GetName().C_Str();
					texture->path = path.C_Str();
					m->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), texture->mapmodeU);
					m->Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), texture->mapmodeV);
					texture->image = Worker::addWork([t = scene->GetEmbeddedTexture(path.C_Str())]() {
						return std::make_unique<Image>(
								std::span {reinterpret_cast<unsigned char *>(t->pcData), t->mWidth}, STBI_rgb_alpha);
					});
					return AssetFactory::TextureFragments::value_type {texture->id, texture};
				});
	}
}

bool
AssImp::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(path);
}
