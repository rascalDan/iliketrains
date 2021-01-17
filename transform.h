#ifndef TRANSFORM_INCLUDED_H
#define TRANSFORM_INCLUDED_H

#include <glm/glm.hpp>
#include <utility>

class Camera;

class Transform {
public:
	Transform(glm::vec3 pos = {}, glm::vec3 rot = {}, glm::vec3 scale = {1.0f, 1.0f, 1.0f});

	glm::mat4 GetModel() const;

	glm::mat4 GetMVP(const Camera & camera) const;

	inline glm::vec3 &
	GetPos()
	{
		return pos;
	}

	inline glm::vec3 &
	GetRot()
	{
		return rot;
	}

	inline glm::vec3 &
	GetScale()
	{
		return scale;
	}

	inline void
	SetPos(glm::vec3 && pos)
	{
		this->pos = std::move(pos);
	}

	inline void
	SetRot(glm::vec3 && rot)
	{
		this->rot = std::move(rot);
	}

	inline void
	SetScale(glm::vec3 && scale)
	{
		this->scale = std::move(scale);
	}

private:
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
};

#endif
