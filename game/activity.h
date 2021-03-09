#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <game/worldobject.h>
#include <memory>
#include <special_members.hpp>

class Vehicle;

class Activity {
public:
	Activity() = default;
	DEFAULT_MOVE_COPY(Activity);
	virtual ~Activity() = default;

	virtual void apply(Vehicle *, TickDuration) const = 0;

	template<typename T> class Of;
};
using ActivityPtr = std::unique_ptr<Activity>;

template<typename T> concept ActivityConcept = std::is_base_of_v<Activity, T>;
template<ActivityConcept AC> class Can {
public:
	virtual void doActivity(const AC *, TickDuration) = 0;
};

#endif
