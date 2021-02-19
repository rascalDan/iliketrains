#define BOOST_TEST_MODULE test_obj

#include <boost/test/unit_test.hpp>

#include <gfx/models/obj.h>
#include <memory>
#include <vector>

BOOST_AUTO_TEST_CASE(objparse)
{
	ObjParser op {"/home/randomdan/dev/game/res/brush47.obj"};
	BOOST_REQUIRE_EQUAL(0, op.yylex());
	BOOST_CHECK_EQUAL(48, op.vertices.size());
	BOOST_CHECK_EQUAL(104, op.texCoords.size());
	BOOST_CHECK_EQUAL(25, op.normals.size());
	BOOST_CHECK_EQUAL(3, op.objects.size());
	const auto & object {op.objects.front()};
	BOOST_CHECK_EQUAL("Body", object.first);
	BOOST_CHECK_EQUAL(18, object.second.size());
	BOOST_CHECK_EQUAL(6, object.second[0].size());
	BOOST_CHECK_EQUAL(6, object.second[1].size());
	BOOST_CHECK_EQUAL(4, object.second[12].size());
}

BOOST_AUTO_TEST_CASE(create_meshes)
{
	ObjParser op {"/home/randomdan/dev/game/res/brush47.obj"};
	const auto ms = op.createMeshes();
	BOOST_REQUIRE_EQUAL(3, ms.size());
}
