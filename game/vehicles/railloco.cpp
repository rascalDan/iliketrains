#include "railloco.h"
#include "gfx/gl/shader.h"
#include "gfx/models/obj.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <filesystem>
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

void
RailVehicle::render(const Shader & shader) const
{
	texture->Bind();
	for (const auto & bogie : bogies) {
		shader.setModel(bogie.location);
		bogie.mesh->Draw();
	}
	shader.setModel(location);
	bodyMesh->Draw();
}

void
RailLoco::move(TickDuration dur)
{
	static std::mt19937 gen(std::random_device {}());
	linkDist += dur.count() * speed;
	auto curLink {linkHist.getCurrent()};
	while (linkDist > curLink.first->length) {
		location = curLink.first->positionAt(curLink.first->length, curLink.second);
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

Location
RailLoco::getBogiePosition(float linkDist, float dist) const
{
	float b2linkDist {};
	const auto b2Link = linkHist.getAt(dist - linkDist, &b2linkDist);
	return b2Link.first->positionAt(b2linkDist, b2Link.second);
}

void
RailLoco::updateRailVehiclePosition(RailVehicle * w, float trailBy) const
{
	const auto overhang {(w->length - w->wheelBase) / 2};
	const auto & b1Pos = w->bogies[0].location = getBogiePosition(linkDist, trailBy += overhang);
	const auto & b2Pos = w->bogies[1].location = getBogiePosition(linkDist, trailBy + wheelBase);
	const auto diff = glm::normalize(b2Pos.pos - b1Pos.pos);
	w->location.pos = (b1Pos.pos + b2Pos.pos) / 2.F;
	w->location.rot = {-vector_pitch(diff), vector_yaw(diff), 0};
}

void
RailLoco::tick(TickDuration dur)
{
	move(dur);
	updateRailVehiclePosition(this, 0);
	updateWagons();
}

void
RailLoco::updateWagons() const
{
	// Drag wagons
	float trailBy {length};
	for (const auto & wagon : wagons) {
		const auto w {wagon.lock()};
		updateRailVehiclePosition(w.get(), trailBy);
		trailBy += w->length;
	}
}

void RailWagon::tick(TickDuration) { }

void
bogieOffset(ObjParser & o)
{
	// offset bogie positions so they can be set directly
	for (int b = 1; b < 3; b++) { // bogie object index
		std::set<std::pair<float, int>> vertexIds;
		for (const auto & face : o.objects[b].second) {
			for (const auto & faceElement : face) {
				vertexIds.emplace(o.vertices[faceElement.x - 1].z, faceElement.x - 1);
			}
		}
		auto offset = (vertexIds.begin()->first + vertexIds.rbegin()->first) / 2;
		for (const auto & v : vertexIds) {
			o.vertices[v.second].z -= offset;
		}
	}
}

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, 0)
{
	ObjParser o {Resource::mapPath("brush47.obj")};
	bogieOffset(o);
	const auto m = o.createMeshes();
	bodyMesh = m.at("Body");
	bogies[0].mesh = m.at("Bogie1");
	bogies[1].mesh = m.at("Bogie2");
	texture = Texture::cachedTexture.get(Resource::mapPath("brush47.png"));
	wheelBase = 12.F;
	length = 20.F;
	speed = 33.6F;
	linkDist = wheelBase;
}

Brush47Wagon::Brush47Wagon(const LinkPtr & l) : RailWagon(l, 0)
{
	ObjParser o {Resource::mapPath("brush47.obj")};
	bogieOffset(o);
	const auto m = o.createMeshes();
	bodyMesh = m.at("Body");
	bogies[0].mesh = m.at("Bogie1");
	bogies[1].mesh = m.at("Bogie2");
	texture = Texture::cachedTexture.get(Resource::mapPath("brush47.png"));
	wheelBase = 12.F;
	length = 20.F;
	linkDist = wheelBase;
}
