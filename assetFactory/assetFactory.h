#pragma once

#include "persistence.h"
#include "shape.h"
#include <filesystem>

class AssetFactory : public Persistence::Persistable {
public:
	using Shapes = std::map<std::string, Shape::CPtr, std::less<>>;

	AssetFactory();
	[[nodiscard]] static std::shared_ptr<AssetFactory> loadXML(const std::filesystem::path &);

	Shapes shapes;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<AssetFactory>, true>;
	bool persist(Persistence::PersistenceStore & store) override;
};
