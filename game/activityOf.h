#ifndef ACTIVITYOF_H
#define ACTIVITYOF_H

#include "activity.h"
#include "vehicles/vehicle.h"

template<typename T> class Activity::Of : public Activity {
	void
	apply(Vehicle * v, TickDuration dur) const override
	{
		if (auto tv = dynamic_cast<Can<T> *>(v)) {
			tv->doActivity(static_cast<const T *>(this), dur);
		}
	}
};

#endif
