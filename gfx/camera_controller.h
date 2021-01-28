#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <game/worldobject.h>

class Camera;
class Shader;

class CameraController : public WorldObject {
public:
	virtual void updateCamera(Camera *, Shader *) const = 0;
};

#endif
