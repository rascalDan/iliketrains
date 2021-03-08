#ifndef ORDERS_H
#define ORDERS_H

#include <collection.hpp>
#include <memory>

class Objective;

class Orders : public Collection<Objective> {
public:
	[[nodiscard]] Objective * current() const;
	Objective * next();
};
using OrdersPtr = std::shared_ptr<Orders>;

#endif
