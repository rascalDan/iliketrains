#ifndef UICOMPONENT_H
#define UICOMPONENT_H

class UIShader;

class UIComponent {
public:
	virtual ~UIComponent() = default;
	virtual void render(const UIShader &) const = 0;
};

#endif
