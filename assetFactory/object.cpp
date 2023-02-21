#include "object.h"
#include <algorithm>

Object::Object(std::string i) : id {std::move(i)} { }

Object::CreatedFaces
Object::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	CreatedFaces faces;
	for (const auto & use : uses) {
		auto useFaces = use->createMesh(mesh, mutation);
		std::transform(useFaces.begin(), useFaces.end(), std::inserter(faces, faces.end()), [this](auto && face) {
			return std::make_pair(id + ":" + face.first, std::move(face.second));
		});
	}
	return faces;
}

template<typename Container, typename Type> struct Appender : public Persistence::SelectionT<std::shared_ptr<Type>> {
	Appender(Container & c) : Persistence::SelectionT<std::shared_ptr<Type>> {s}, container {c} { }
	using Persistence::SelectionT<std::shared_ptr<Type>>::SelectionT;
	void
	endObject(Persistence::Stack & stk) override
	{
		container.emplace_back(s);
		stk.pop();
	}

private:
	std::shared_ptr<Type> s;
	Container & container;
};

bool
Object::persist(Persistence::PersistenceStore & store)
{
	using UseAppend = Appender<Use::Collection, Use>;
	return STORE_TYPE && STORE_MEMBER(id) && STORE_NAME_HELPER("use", uses, UseAppend);
}
