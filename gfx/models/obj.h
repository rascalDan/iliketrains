#ifndef OBJ_H
#define OBJ_H

#ifndef yyFlexLexer
#	define yyFlexLexer objbaseFlexLexer
#	include <FlexLexer.h>
#endif
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <vector>

class Mesh;
class Vertex;

class ObjParser : yyFlexLexer {
public:
	explicit ObjParser(const std::filesystem::path & fileName);
	explicit ObjParser(std::unique_ptr<std::istream> in);

	int yylex() override;

	std::vector<glm::vec4> vertices;
	std::vector<glm::vec3> texCoords;
	std::vector<glm::vec3> normals;
	using FaceElement = glm::vec<3, int>;
	using Face = std::vector<FaceElement>;
	using Faces = std::vector<Face>;
	using Object = std::pair<std::string, Faces>;
	std::vector<Object> objects;
	glm::length_t axis {0};

	using NamedMeshesData = std::map<std::string, std::pair<std::vector<Vertex>, std::vector<unsigned int>>>;
	[[nodiscard]] NamedMeshesData createMeshData() const;
	using NamedMeshes = std::map<std::string, std::shared_ptr<const Mesh>>;
	[[nodiscard]] NamedMeshes createMeshes() const;
};

#endif
