#define BOOST_TEST_MODULE test_model_factory

#include "testHelpers.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "assetFactory/factoryMesh.h"
#include "assetFactory/modelFactory.h"
#include "assetFactory/object.h"
#include "gfx/gl/sceneRenderer.h"
#include "lib/collection.hpp"
#include "lib/location.hpp"
#include "testMainWindow.h"
#include "ui/applicationBase.h"

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

const std::filesystem::path TMP {"/tmp"};
class FactoryFixture : public TestRenderOutputSize<glm::ivec2 {2048, 2048}>, public SceneProvider {
public:
	FactoryFixture() : sceneRenderer {size, output} { }
	~FactoryFixture()
	{
		glDisable(GL_DEBUG_OUTPUT);
		auto outpath = (TMP / boost::unit_test::framework::current_test_case().full_name()).replace_extension(".tga");
		std::filesystem::create_directories(outpath.parent_path());
		Texture::save(outImage, size, outpath.c_str());
	}
	void
	content(const SceneShader & shader) const override
	{
		shader.basic.use(Location {{0, 0, 0}, {0, 0, 0}});
		meshes.apply(&Mesh::Draw);
	}
	void
	lights(const SceneShader & shader) const override
	{
		shader.pointLight.add({-3, 1, 5}, {1, 1, 1}, .1F);
	}
	void
	environment(const SceneShader &, const SceneRenderer & sceneRenderer) const override
	{
		sceneRenderer.setAmbientLight({.2, .2, .2});
		sceneRenderer.setDirectionalLight({.3, .3, .3}, east + south + south + down, *this);
	}
	void
	shadows(const ShadowMapper & mapper) const override
	{
		mapper.dynamicPoint.use(Location {{0, 0, 0}, {0, 0, 0}});
		meshes.apply(&Mesh::Draw);
	}
	void
	render(float dist = 10.f)
	{
		sceneRenderer.camera.setView({dist, dist, dist}, south + west + down);
		sceneRenderer.render(*this);
	}
	Collection<const Mesh> meshes;

private:
	SceneRenderer sceneRenderer;
};

BOOST_FIXTURE_TEST_SUITE(m, FactoryFixture);
BOOST_AUTO_TEST_CASE(brush47)
{
	ModelFactory modelFactory;
	{
		auto wheel = std::make_shared<Object>("wheel");
		{
			auto wheelCylinder = wheel->uses.emplace_back(std::make_shared<Use>());
			wheelCylinder->type = modelFactory.shapes.at("cylinder");
			wheelCylinder->position = {0, 0, 0.571};
			wheelCylinder->scale = {1.142, 1.142, 0.07};
			wheelCylinder->rotation = {0, 0, half_pi};
			wheelCylinder->colour = "#2C3539";
		}
		modelFactory.shapes.emplace(wheel->id, wheel);
	}
	{
		auto axel = std::make_shared<Object>("axel");
		for (float x : {-1.f, 1.f}) {
			auto wheel = axel->uses.emplace_back(std::make_shared<Use>());
			wheel->type = modelFactory.shapes.at("wheel");
			wheel->position = {x * 0.717f, 0, 0};
			wheel->rotation = {0, x == 1.f ? pi : 0.f, 0};
		}
		modelFactory.shapes.emplace(axel->id, axel);
	}
	{
		auto bogey = std::make_shared<Object>("bogey");
		for (float y : {-2.f, 0.f, 2.f}) {
			auto axel = bogey->uses.emplace_back(std::make_shared<Use>());
			axel->type = modelFactory.shapes.at("axel");
			axel->position = {0, y, 0};
		}
		modelFactory.shapes.emplace(bogey->id, bogey);
	}
	FactoryMesh::Collection factoryMeshes;
	{
		unsigned short b {0};
		for (float y : {-6.f, 6.f}) {
			auto bogey = factoryMeshes.emplace_back(std::make_shared<FactoryMesh>());
			bogey->id = "bogey" + std::to_string(b);
			auto bogeyUse = bogey->uses.emplace_back(std::make_shared<Use>());
			bogeyUse->type = modelFactory.shapes.at("bogey");
			bogeyUse->position = {0, y, 0};
			bogeyUse->rotation = {0, b * pi, 0};
			b++;
		}
	}
	{
		auto body = factoryMeshes.emplace_back(std::make_shared<FactoryMesh>());
		body->id = "body";
		body->size = {2.69f, 19.38f, 3.9f};
		{
			auto bodyLower = body->uses.emplace_back(std::make_shared<Use>());
			bodyLower->type = modelFactory.shapes.at("cuboid");
			bodyLower->position = {0, 0, 1.2};
			bodyLower->scale = {2.69, 19.38, 1.5};
			bodyLower->colour = "#1111DD";
			bodyLower->faceControllers["bottom"].colour = "#2C3539";
			auto & bodyUpper = bodyLower->faceControllers["top"];
			bodyUpper.type = "extrude";
			bodyUpper.scale = {1, .95f, 1};
			bodyUpper.position = {0, 0, 1.0};
			auto & roof = bodyUpper.faceControllers["top"];
			roof.type = "extrude";
			roof.scale = {.6f, .9f, 0};
			roof.position = {0, 0, 0.2};
			roof.smooth = true;
		}
		{
			auto batteryBox = body->uses.emplace_back(std::make_shared<Use>());
			batteryBox->type = modelFactory.shapes.at("cuboid");
			batteryBox->position = {0, 0, .2};
			batteryBox->scale = {2.6, 4.5, 1};
			batteryBox->colour = "#2C3539";
		}
	}
	std::transform(factoryMeshes.begin(), factoryMeshes.end(), std::back_inserter(meshes.objects),
			[](const FactoryMesh::CPtr & factoryMesh) -> Mesh::Ptr {
				return factoryMesh->createMesh();
			});

	render(20);
}
BOOST_AUTO_TEST_SUITE_END();
