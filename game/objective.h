#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "activity.h"
#include <memory>
#include <special_members.hpp>

class Orders;

class Objective {
public:
	explicit Objective(Orders * os) : orders(os) { }
	DEFAULT_MOVE_COPY(Objective);
	virtual ~Objective() = default;

	[[nodiscard]] virtual Objective * complete() const;
	[[nodiscard]] virtual ActivityPtr createActivity() const = 0;

	Orders * orders;
};
using ObjectivePtr = std::unique_ptr<Objective>;

#endif
