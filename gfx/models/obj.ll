%option batch
%option c++
%option noyywrap
%option 8bit
%option stack
%option yyclass="ObjParser"
%option prefix="objbase"

%{
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#if __clang__
#pragma GCC diagnostic ignored "-Wnull-conversion"
#endif
#include <gfx/models/obj.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
class objbaseFlexLexer;
%}

comment #.*
float -?[0-9.]+
index -?[0-9]+
linestring [^\r\n]*
truthy (1|on)
falsey (0|off)

%x FACE
%x MTLLIB
%x OBJECT
%x SMOOTH
%x USEMTL
%x VERTEX
%x NORMAL
%x TEXCOORD

%%

<INITIAL>{comment} {
  // fprintf(stderr, "COMMENT %s\n", YYText());
}
<INITIAL>"f " {
  BEGIN(FACE);
  objects.back().second.emplace_back();
  objects.back().second.back().emplace_back();
  axis = 0;
}
<INITIAL>"mtllib " {
  BEGIN(MTLLIB);
}
<INITIAL>"o " {
  BEGIN(OBJECT);
}
<INITIAL>"s " {
  BEGIN(SMOOTH);
}
<INITIAL>"usemtl " {
  BEGIN(USEMTL);
}
<INITIAL>"v " {
  BEGIN(VERTEX);
  vertices.emplace_back(0, 0, 0, 1);
  axis = 0;
}
<INITIAL>"vn " {
  BEGIN(NORMAL);
  normals.emplace_back(0, 0, 0);
  axis = 0;
}
<INITIAL>"vt " {
  BEGIN(TEXCOORD);
  texCoords.emplace_back(0, 1, 1);
  axis = 0;
}
<USEMTL>{linestring} {
  // fprintf(stderr, "USEMTL <%s>\n", YYText());
}
<MTLLIB>{linestring} {
  // fprintf(stderr, "MTLLIB <%s>\n", YYText());
}
<OBJECT>{linestring} {
  objects.emplace_back(YYText(), Faces{});
}
<SMOOTH>{truthy} {
  // fprintf(stderr, "Set smooth\n");
}
<SMOOTH>{falsey} {
  // fprintf(stderr, "Set flat\n");
}
<VERTEX>{float} {
  vertices.back()[axis++] = std::stof(YYText());
}
<NORMAL>{float} {
  normals.back()[axis++] = std::stof(YYText());
}
<TEXCOORD>{float} {
  texCoords.back()[axis++] = std::stof(YYText());
}
<FACE>{index} {
  objects.back().second.back().back()[axis] = std::stoi(YYText());
}
<FACE>\/ {
  axis++;
}
<FACE>[ \t] {
  objects.back().second.back().emplace_back();
  axis = 0;
}

<*>[ \t] {
}
<*>[\r\n\f] {
  BEGIN(INITIAL);
}

%%

// Make iwyu think unistd.h is required
[[maybe_unused]]static auto x=getpid;
