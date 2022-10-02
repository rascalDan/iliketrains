#pragma once

class Shader;

class WorldOverlay {
public:
	virtual ~WorldOverlay() = default;
	virtual void render(const Shader &) const = 0;
};
