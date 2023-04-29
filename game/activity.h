#pragma once

#include <game/worldobject.h>
#include <memory>
#include <special_members.h>

class Vehicle;

class Activity {
public:
	Activity() = default;
	DEFAULT_MOVE_COPY(Activity);
	virtual ~Activity() = default;

	virtual void apply(Vehicle *, TickDuration) = 0;

	template<typename T> class Of;
};
using ActivityPtr = std::unique_ptr<Activity>;

template<typename T>
concept ActivityConcept = std::is_base_of_v<Activity, T>;
template<ActivityConcept AC> class Can {
public:
	Can() = default;
	virtual ~Can() = default;
	DEFAULT_MOVE_COPY(Can);

	virtual void doActivity(AC *, TickDuration) = 0;
};
