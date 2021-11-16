#define BOOST_TEST_MODULE test_obj

#include <boost/test/unit_test.hpp>

#include <gfx/models/obj.h>
#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <utility>
#include <vector>

BOOST_AUTO_TEST_CASE(objparse)
{
	ObjParser op {RESDIR "/brush47.obj"};
	BOOST_CHECK_EQUAL(75, op.vertices.size());
	BOOST_CHECK_EQUAL(112, op.texCoords.size());
	BOOST_CHECK_EQUAL(31, op.normals.size());
	BOOST_CHECK_EQUAL(3, op.objects.size());
	const auto & object {op.objects.front()};
	BOOST_CHECK_EQUAL("Body", object.first);
	BOOST_CHECK_EQUAL(21, object.second.size());
	BOOST_CHECK_EQUAL(8, object.second[0].size());
	BOOST_CHECK_EQUAL(8, object.second[1].size());
	BOOST_CHECK_EQUAL(4, object.second[12].size());
}

BOOST_AUTO_TEST_CASE(create_meshes)
{
	ObjParser op {RESDIR "/brush47.obj"};
	const auto ms = op.createMeshData();
	BOOST_REQUIRE_EQUAL(3, ms.size());
	BOOST_REQUIRE_EQUAL("Body", ms.begin()->first);
	const auto & o = ms.at("Body");
	BOOST_REQUIRE_EQUAL(88, o.first.size());
	const auto & v = o.first.front();
	BOOST_REQUIRE_CLOSE(-1.345, v.pos.x, 1);
	BOOST_REQUIRE_EQUAL(138, o.second.size());
}
