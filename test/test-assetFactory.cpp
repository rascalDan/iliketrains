#define BOOST_TEST_MODULE test_asset_factory

#include "testHelpers.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "assetFactory/assetFactory.h"
#include "assetFactory/factoryMesh.h"
#include "assetFactory/object.h"
#include "gfx/gl/sceneRenderer.h"
#include "lib/collection.hpp"
#include "lib/location.hpp"
#include "lib/stream_support.hpp"
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
	AssetFactory assetFactory;
	{
		auto wheel = std::make_shared<Object>("wheel");
		{
			auto wheelCylinder = wheel->uses.emplace_back(std::make_shared<Use>());
			wheelCylinder->type = assetFactory.shapes.at("cylinder");
			wheelCylinder->position = {0, 0, 0.571};
			wheelCylinder->scale = {1.142, 1.142, 0.07};
			wheelCylinder->rotation = {0, 0, half_pi};
			wheelCylinder->colour = assetFactory.parseColour("#2C3539");
		}
		assetFactory.shapes.emplace(wheel->id, wheel);
	}
	{
		auto axel = std::make_shared<Object>("axel");
		for (float x : {-1.f, 1.f}) {
			auto wheel = axel->uses.emplace_back(std::make_shared<Use>());
			wheel->type = assetFactory.shapes.at("wheel");
			wheel->position = {x * 0.717f, 0, 0};
			wheel->rotation = {0, x == 1.f ? pi : 0.f, 0};
		}
		assetFactory.shapes.emplace(axel->id, axel);
	}
	{
		auto bogie = std::make_shared<Object>("bogie");
		for (float y : {-2.f, 0.f, 2.f}) {
			auto axel = bogie->uses.emplace_back(std::make_shared<Use>());
			axel->type = assetFactory.shapes.at("axel");
			axel->position = {0, y, 0};
		}
		assetFactory.shapes.emplace(bogie->id, bogie);
	}
	FactoryMesh::Collection factoryMeshes;
	{
		unsigned short b {0};
		for (float y : {-6.f, 6.f}) {
			auto bogie = factoryMeshes.emplace_back(std::make_shared<FactoryMesh>());
			bogie->id = "bogie" + std::to_string(b);
			auto bogieUse = bogie->uses.emplace_back(std::make_shared<Use>());
			bogieUse->type = assetFactory.shapes.at("bogie");
			bogieUse->position = {0, y, 0};
			bogieUse->rotation = {0, b * pi, 0};
			b++;
		}
	}
	{
		auto body = factoryMeshes.emplace_back(std::make_shared<FactoryMesh>());
		body->id = "body";
		body->size = {2.69f, 19.38f, 3.9f};
		auto bodyLower = body->uses.emplace_back(std::make_shared<Use>());
		bodyLower->type = assetFactory.shapes.at("cuboid");
		bodyLower->position = {0, 0, 1.2};
		bodyLower->scale = {2.69, 19.38, 1.5};
		bodyLower->colour = assetFactory.parseColour("#1111DD");
		auto & bottom = bodyLower->faceControllers["bottom"];
		bottom = std::make_unique<FaceController>();
		bottom->colour = assetFactory.parseColour("#2C3539");
		auto & bodyUpper = bodyLower->faceControllers["top"];
		bodyUpper = std::make_unique<FaceController>();
		bodyUpper->type = "extrude";
		bodyUpper->scale = {1, .95f, 1};
		bodyUpper->position = {0, 0, 1.0};
		auto & roof = bodyUpper->faceControllers["top"];
		roof = std::make_unique<FaceController>();
		roof->type = "extrude";
		roof->scale = {.6f, .9f, 0};
		roof->position = {0, 0, 0.2};
		roof->smooth = true;
		{
			auto batteryBox = body->uses.emplace_back(std::make_shared<Use>());
			batteryBox->type = assetFactory.shapes.at("cuboid");
			batteryBox->position = {0, 0, .2};
			batteryBox->scale = {2.6, 4.5, 1};
			batteryBox->colour = assetFactory.parseColour("#2C3539");
		}
	}
	std::transform(factoryMeshes.begin(), factoryMeshes.end(), std::back_inserter(meshes.objects),
			[](const FactoryMesh::CPtr & factoryMesh) -> Mesh::Ptr {
				return factoryMesh->createMesh();
			});

	render(20);
}
BOOST_AUTO_TEST_CASE(brush47xml)
{
	auto mf = AssetFactory::loadXML(RESDIR "/brush47.xml");
	BOOST_REQUIRE(mf);
	BOOST_REQUIRE_EQUAL(6, mf->shapes.size());
	BOOST_CHECK(mf->shapes.at("plane"));
	BOOST_CHECK(mf->shapes.at("cylinder"));
	BOOST_CHECK(mf->shapes.at("cuboid"));
	BOOST_CHECK(mf->shapes.at("wheel"));
	BOOST_CHECK(mf->shapes.at("axel"));
	auto bogie = mf->shapes.at("bogie");
	BOOST_REQUIRE(bogie);
	auto bogieObj = std::dynamic_pointer_cast<const Object>(bogie);
	BOOST_CHECK_EQUAL(3, bogieObj->uses.size());
	BOOST_CHECK_EQUAL(1, mf->assets.size());
	auto brush47 = mf->assets.at("brush-47");
	BOOST_REQUIRE(brush47);
	BOOST_CHECK_EQUAL(3, brush47->meshes.size());
	auto body = brush47->meshes.at(0);
	BOOST_REQUIRE(body);
	BOOST_CHECK_EQUAL("body", body->id);
	BOOST_CHECK_EQUAL(2, body->uses.size());

	std::transform(brush47->meshes.begin(), brush47->meshes.end(), std::back_inserter(meshes.objects),
			[](const FactoryMesh::CPtr & factoryMesh) -> Mesh::Ptr {
				return factoryMesh->createMesh();
			});

	render(20);
}
BOOST_AUTO_TEST_SUITE_END();

template<typename T> using InOut = std::tuple<T, T>;
BOOST_DATA_TEST_CASE(normalizeColourName,
		boost::unit_test::data::make<InOut<std::string>>({
				{"", ""},
				{"black", "black"},
				{" black ", "black"},
				{" b l a c k ", "black"},
				{" B L A c k ", "black"},
				{"BLAck ", "black"},
				{"BLACK ", "black"},
				{"BlAck ", "black"},
				{"Bl Ack ", "black"},
		}),
		in_, exp)
{
	auto in {in_};
	BOOST_CHECK_NO_THROW(AssetFactory::normalizeColourName(in));
	BOOST_CHECK_EQUAL(in, exp);
}

BOOST_AUTO_TEST_CASE(parseX11RGB)
{
	const auto parsedColours = AssetFactory::parseX11RGB(FIXTURESDIR "rgb.txt");
	BOOST_REQUIRE_EQUAL(parsedColours.size(), 20);
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("cyan"), AssetFactory::Colour(0, 1, 1));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("slategrey"), AssetFactory::Colour(0.44F, 0.5, 0.56F));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("lightsteelblue1"), AssetFactory::Colour(0.79, 0.88, 1));
}
