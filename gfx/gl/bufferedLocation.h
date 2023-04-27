#pragma once

#include "instanceVertices.h"
#include "location.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class BufferedLocation {
public:
	BufferedLocation(InstanceVertices<glm::mat4> &, glm::vec3 = {}, glm::vec3 = {});
	BufferedLocation(InstanceVertices<glm::mat4> &, const Location &);

	BufferedLocation & operator=(const Location &);

	operator const Location &() const;

	glm::vec3 position() const;
	glm::vec3 rotation() const;
	void setPosition(glm::vec3, bool update = true);
	void setRotation(glm::vec3, bool update = true);
	void setLocation(glm::vec3, glm::vec3);

	glm::mat4 getTransform() const;

private:
	void updateBuffer();

	Location loc;
	InstanceVertices<glm::mat4>::InstanceProxy buffer;
};
