#ifndef RENDERABLE_H
#define RENDERABLE_H

class Shader;

class Renderable {
public:
	virtual void render(const Shader & shader) const = 0;
};

#endif
