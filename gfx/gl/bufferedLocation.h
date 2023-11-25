#pragma once

#include "location.h"
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <utility>

class BufferedLocation {
public:
	BufferedLocation(GlobalPosition3D = {}, Rotation3D = {});
	BufferedLocation(const Location &);
	virtual ~BufferedLocation() = default;

	BufferedLocation & operator=(const Location &);

	operator const Location &() const;

	[[nodiscard]] GlobalPosition3D position() const;
	[[nodiscard]] Rotation3D rotation() const;
	void setPosition(GlobalPosition3D, bool update = true);
	void setRotation(Rotation3D, bool update = true);
	void setLocation(GlobalPosition3D, Rotation3D);

	[[nodiscard]] glm::mat4 getRotationTransform() const;

private:
	virtual void updateBuffer() const = 0;

	Location loc;
};

class BufferedLocationUpdater : public BufferedLocation {
public:
	template<typename... LocationArgs>
	BufferedLocationUpdater(std::function<void(const BufferedLocation *)> onUpdate, LocationArgs &&... t) :
		BufferedLocation {std::forward<LocationArgs>(t)...}, onUpdate {std::move(onUpdate)}
	{
		updateBuffer();
	}

	using BufferedLocation::operator=;

private:
	void updateBuffer() const override;

	std::function<void(const BufferedLocation *)> onUpdate;
};
