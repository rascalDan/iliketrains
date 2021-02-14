#ifndef TRANSFORM_INCLUDED_H
#define TRANSFORM_INCLUDED_H

#include <glm/glm.hpp>

class Transform {
public:
	explicit Transform(glm::vec3 pos = {}, glm::vec3 rot = {});

	[[nodiscard]] glm::mat4 GetModel() const;

	[[nodiscard]] inline glm::vec3 &
	GetPos()
	{
		return pos;
	}

	[[nodiscard]] inline const glm::vec3 &
	GetPos() const
	{
		return pos;
	}

	[[nodiscard]] inline glm::vec3 &
	GetRot()
	{
		return rot;
	}

	[[nodiscard]] inline const glm::vec3 &
	GetRot() const
	{
		return rot;
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

private:
	glm::vec3 pos;
	glm::vec3 rot;
};

#endif
