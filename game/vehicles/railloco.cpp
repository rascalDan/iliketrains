#include "railloco.h"
#include "gfx/gl/shader.h"
#include "gfx/gl/transform.h"
#include "gfx/models/obj.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <glm/glm.hpp>
#include <lib/resource.h>
#include <maths.h>
#include <memory>
#include <set>
#include <utility>
#include <vector>

void
RailVehicle::render(const Shader & shader) const
{
	shader.setModel(location.GetModel());
	texture->Bind();
	bodyMesh->Draw();
	for (const auto & bogey : bogeys) {
		shader.setModel(bogey.location.GetModel());
		bogey.mesh->Draw();
	}
}

void
RailLoco::move(TickDuration dur)
{
	linkDist += dur.count() * speed;
	auto curLink {linkHist.getCurrent()};
	while (linkDist > curLink.first->length) {
		location = curLink.first->positionAt(curLink.first->length, curLink.second);
		const auto & nexts {curLink.first->nexts[1 - curLink.second]};
		const auto next = std::find_if(nexts.begin(), nexts.end(), [ang = location.GetRot().y](const Link::Next & n) {
			return std::abs(normalize(n.first.lock()->ends[n.second].second - ang)) < 0.1F;
		});
		if (next != nexts.end()) {
			linkDist -= curLink.first->length;
			curLink = linkHist.add(next->first, next->second);
		}
		else {
			linkDist = curLink.first->length;
			speed = 0;
		}
	}
}

Transform
RailLoco::getBogeyPosition(float linkDist, float dist) const
{
	float b2linkDist {};
	const auto b2Link = linkHist.getAt(dist - linkDist, &b2linkDist);
	return b2Link.first->positionAt(b2linkDist, b2Link.second);
}

void
RailLoco::updateRailVehiclePosition(RailVehicle * w, float trailBy) const
{
	const auto overhang {(w->length - w->wheelBase) / 2};
	const auto & b1Pos = w->bogeys[0].location = getBogeyPosition(linkDist, trailBy += overhang);
	const auto & b2Pos = w->bogeys[1].location = getBogeyPosition(linkDist, trailBy += wheelBase);
	const auto diff = glm::normalize(b2Pos.GetPos() - b1Pos.GetPos());
	w->location.GetPos() = (b1Pos.GetPos() + b2Pos.GetPos()) / 2.F;
	w->location.GetRot() = {-vector_pitch(diff), vector_yaw(diff), 0};
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
bogeyOffset(ObjParser & o)
{
	// offset bogey positions so they can be set directly
	for (int b = 1; b < 3; b++) { // bogey object index
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
	bogeyOffset(o);
	const auto m = o.createMeshes();
	bodyMesh = m[0].second;
	bogeys[0].mesh = m[1].second;
	bogeys[1].mesh = m[2].second;
	texture = Texture::cachedTexture.get(Resource::mapPath("brush47.png"));
	wheelBase = 12.F;
	length = 20.F;
	linkDist = wheelBase;
}

Brush47Wagon::Brush47Wagon(const LinkPtr & l) : RailWagon(l, 0)
{
	ObjParser o {Resource::mapPath("brush47.obj")};
	bogeyOffset(o);
	const auto m = o.createMeshes();
	bodyMesh = m[0].second;
	bogeys[0].mesh = m[1].second;
	bogeys[1].mesh = m[2].second;
	texture = Texture::cachedTexture.get(Resource::mapPath("brush47.png"));
	wheelBase = 12.F;
	length = 20.F;
	linkDist = wheelBase;
}
