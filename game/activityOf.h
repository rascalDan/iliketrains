#pragma once

#include "activity.h"
#include "vehicles/vehicle.h"

template<typename T> class Activity::Of : public Activity {
	void
	apply(Vehicle * v, TickDuration dur) override
	{
		if (auto tv = dynamic_cast<Can<T> *>(v)) {
			tv->doActivity(static_cast<T *>(this), dur);
		}
	}
};
