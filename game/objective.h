#pragma once

#include "activity.h"
#include "network/link.h"
#include <memory>
#include <special_members.h>

class Orders;

class Objective {
public:
	explicit Objective(Orders * os) : orders(os) { }

	DEFAULT_MOVE_COPY(Objective);
	virtual ~Objective() = default;

	[[nodiscard]] virtual Objective * complete() const;
	[[nodiscard]] virtual ActivityPtr createActivity() const = 0;
	[[nodiscard]] virtual Link::Next navigate(Link::Nexts::const_iterator, Link::Nexts::const_iterator) const = 0;

	Orders * orders;
};

using ObjectivePtr = std::unique_ptr<Objective>;
