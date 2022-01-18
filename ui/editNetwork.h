#pragma once

#include "gameMainSelector.h"

class Ray;

class EditNetwork : public GameMainSelector::ClickReceiver {
public:
	void click(const Ray &) override;
	void move(const Ray &) override;
	bool handleMove() override;
};
