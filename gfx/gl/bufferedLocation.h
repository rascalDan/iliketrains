#pragma once

#include "location.h"
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <tuple>

class BufferedLocation {
public:
	BufferedLocation(Position3D = {}, Rotation3D = {});
	BufferedLocation(const Location &);
	virtual ~BufferedLocation() = default;

	BufferedLocation & operator=(const Location &);

	operator const Location &() const;

	[[nodiscard]] Position3D position() const;
	[[nodiscard]] Rotation3D rotation() const;
	void setPosition(Position3D, bool update = true);
	void setRotation(Rotation3D, bool update = true);
	void setLocation(Position3D, Rotation3D);

	[[nodiscard]] glm::mat4 getTransform() const;

private:
	virtual void updateBuffer() = 0;

	Location loc;
};

template<typename... Target> class BufferedLocationT : public BufferedLocation {
public:
	template<typename... LocationArgs>
	BufferedLocationT(Target &&... target, LocationArgs &&... t) :
		BufferedLocation {std::forward<LocationArgs>(t)...}, target {std::forward<Target>(target)...}
	{
		updateBuffer();
	}

	using BufferedLocation::operator=;

private:
	void
	updateBuffer() override
	{
		std::apply(std::invoke<const Target &...>, target) = getTransform();
	}

	std::tuple<Target...> target;
};
