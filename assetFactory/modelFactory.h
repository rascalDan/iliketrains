#pragma once

#include "persistence.h"
#include "shape.h"
#include <filesystem>

class ModelFactory : public Persistence::Persistable {
public:
	using Shapes = std::map<std::string, Shape::CPtr, std::less<>>;

	ModelFactory();
	[[nodiscard]] static std::shared_ptr<ModelFactory> loadXML(const std::filesystem::path &);

	Shapes shapes;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<ModelFactory>, true>;
	bool persist(Persistence::PersistenceStore & store) override;
};
