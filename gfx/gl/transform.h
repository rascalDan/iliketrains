#ifndef TRANSFORM_INCLUDED_H
#define TRANSFORM_INCLUDED_H

#include <glm/glm.hpp>

class Camera;

class Transform {
public:
	explicit Transform(glm::vec3 pos = {}, glm::vec3 rot = {}, glm::vec3 scale = {1.0F, 1.0F, 1.0F});

	[[nodiscard]] glm::mat4 GetModel() const;

	[[nodiscard]] glm::mat4 GetMVP(const Camera & camera) const;

	[[nodiscard]] inline glm::vec3 &
	GetPos()
	{
		return pos;
	}

	[[nodiscard]] inline glm::vec3 &
	GetRot()
	{
		return rot;
	}

	[[nodiscard]] inline glm::vec3 &
	GetScale()
	{
		return scale;
	}

	inline void
	SetPos(glm::vec3 && pos)
	{
		this->pos = pos;
	}

	inline void
	SetRot(glm::vec3 && rot)
	{
		this->rot = rot;
	}

	inline void
	SetScale(glm::vec3 && scale)
	{
		this->scale = scale;
	}

private:
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
};

#endif
