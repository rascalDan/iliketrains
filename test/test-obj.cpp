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
	BOOST_CHECK_EQUAL(16, op.normals.size());
	BOOST_CHECK_EQUAL(28, op.faces.size());
	BOOST_CHECK_EQUAL(4, op.faces[0].size());
	BOOST_CHECK_EQUAL(6, op.faces[10].size());
	BOOST_CHECK_EQUAL(6, op.faces[11].size());
	BOOST_CHECK_EQUAL(4, op.faces[12].size());
}
