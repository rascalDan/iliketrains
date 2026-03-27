#pragma once

#include "config/types.h"
#include "worldobject.h"
#include <chrono>

class SceneRenderer;
class SceneProvider;

class Environment : public WorldObject {
public:
	using WorldTime = std::chrono::utc_time<std::chrono::seconds>;

	Environment();
	void tick(TickDuration elapsed) override;
	void render(const SceneRenderer &, const SceneProvider &) const;
	[[nodiscard]] Direction2D getSunPos() const;
	[[nodiscard]] WorldTime getWorldTime() const;
	[[nodiscard]] static Direction2D getSunPos(Direction2D position, time_t time);

private:
	WorldTime worldTime;
	uint16_t gameTimeScaleFactor;
	glm::vec<2, Angle> earthPos;
};
