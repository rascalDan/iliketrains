#include "railloco.h"
#include "gfx/gl/shader.h"
#include "gfx/models/obj.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <iterator>
#include <lib/resource.h>
#include <location.hpp>
#include <map>
#include <maths.h>
#include <memory>
#include <random>
#include <set>
#include <utility>
#include <vector>

RailVehicleClass::RailVehicleClass(const std::string & name) :
	RailVehicleClass {std::make_unique<ObjParser>(Resource::mapPath(name + ".obj")),
			Texture::cachedTexture.get(Resource::mapPath(name + ".png"))}
{
}

RailVehicleClass::RailVehicleClass(std::unique_ptr<ObjParser> o, std::shared_ptr<Texture> t) : texture {std::move(t)}
{
	wheelBase = bogieOffset(*o);
	length = objectLength(*o);
	const auto m = o->createMeshes();
	bodyMesh = m.at("Body");
	bogies[0] = m.at("Bogie1");
	bogies[1] = m.at("Bogie2");
}

void
RailVehicleClass::render(const Shader & shader, const Location & location, const std::array<Location, 2> & bl) const
{
	texture->Bind();
	for (auto b = 0U; b < bogies.size(); ++b) {
		shader.setModel(bl[b]);
		bogies[b]->Draw();
	}
	shader.setModel(location);
	bodyMesh->Draw();
}

float
RailVehicleClass::bogieOffset(ObjParser & o)
{
	float wheelBase {0};
	// offset bogie positions so they can be set directly
	for (auto & object : o.objects) { // bogie object index
		if (!object.first.starts_with("Bogie")) {
			continue;
		}
		std::set<std::pair<float, int>> vertexIds;
		for (const auto & face : object.second) {
			for (const auto & faceElement : face) {
				vertexIds.emplace(o.vertices[faceElement.x - 1].z, faceElement.x - 1);
			}
		}
		const auto offset = (vertexIds.begin()->first + vertexIds.rbegin()->first) / 2;
		for (const auto & v : vertexIds) {
			o.vertices[v.second].z -= offset;
		}
		wheelBase += std::abs(offset);
	}
	return wheelBase;
}

float
RailVehicleClass::objectLength(ObjParser & o)
{
	const auto mme = std::minmax_element(o.vertices.begin(), o.vertices.end(), [](const auto & v1, const auto & v2) {
		return v1.z < v2.z;
	});
	return mme.second->z - mme.first->z;
}

void
RailVehicle::render(const Shader & shader) const
{
	rvClass->render(shader, location, bogies);
}

void
Train::move(TickDuration dur)
{
	static std::mt19937 gen(std::random_device {}());
	linkDist += dur.count() * speed;
	auto curLink {linkHist.getCurrent()};
	while (linkDist > curLink.first->length) {
		const auto location = curLink.first->positionAt(curLink.first->length, curLink.second);
		auto nexts {curLink.first->nexts[1 - curLink.second]};
		auto last = std::remove_if(nexts.begin(), nexts.end(), [ang = location.rot.y](const Link::Next & n) {
			return std::abs(normalize(n.first.lock()->ends[n.second].second - ang)) > 0.1F;
		});
		if (last != nexts.begin()) {
			auto off = std::uniform_int_distribution<>(0, std::distance(nexts.begin(), last) - 1)(gen);
			linkDist -= curLink.first->length;
			curLink = linkHist.add(nexts[off].first, nexts[off].second);
		}
		else {
			linkDist = curLink.first->length;
			speed = 0;
		}
	}
}

void
Train::render(const Shader & shader) const
{
	apply(&Renderable::render, shader);
}

Location
Train::getBogiePosition(float linkDist, float dist) const
{
	float b2linkDist {};
	const auto b2Link = linkHist.getAt(dist - linkDist, &b2linkDist);
	return b2Link.first->positionAt(b2linkDist, b2Link.second);
}

void
RailVehicle::move(const Train * t, float & trailBy)
{
	const auto overhang {(rvClass->length - rvClass->wheelBase) / 2};
	const auto & b1Pos = bogies[0] = t->getBogiePosition(t->linkDist, trailBy += overhang);
	const auto & b2Pos = bogies[1] = t->getBogiePosition(t->linkDist, trailBy += rvClass->wheelBase);
	const auto diff = glm::normalize(b2Pos.pos - b1Pos.pos);
	location.pos = (b1Pos.pos + b2Pos.pos) / 2.F;
	location.rot = {-vector_pitch(diff), vector_yaw(diff), 0};
	trailBy += 0.6F + overhang;
}

void
Train::tick(TickDuration dur)
{
	move(dur);

	float trailBy {0.F};
	apply(&RailVehicle::move, this, std::ref(trailBy));
}
