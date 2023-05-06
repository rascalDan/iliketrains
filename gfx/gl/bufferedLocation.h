#pragma once

#include "location.h"
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <tuple>

class BufferedLocation {
public:
	BufferedLocation(glm::vec3 = {}, glm::vec3 = {});
	BufferedLocation(const Location &);
	virtual ~BufferedLocation() = default;

	BufferedLocation & operator=(const Location &);

	operator const Location &() const;

	glm::vec3 position() const;
	glm::vec3 rotation() const;
	void setPosition(glm::vec3, bool update = true);
	void setRotation(glm::vec3, bool update = true);
	void setLocation(glm::vec3, glm::vec3);

	glm::mat4 getTransform() const;

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
