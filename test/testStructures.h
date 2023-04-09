#pragma once

#include <persistence.h>

struct AbsObject : public Persistence::Persistable {
	std::string base;

	bool persist(Persistence::PersistenceStore & store) override;

	virtual void dummy() const = 0;
};

struct SubObject : public AbsObject {
	std::string sub;

	bool persist(Persistence::PersistenceStore & store) override;

	void dummy() const override;

	[[nodiscard]] std::string getId() const override;
};

struct SubObject2 : public AbsObject {
	bool persist(Persistence::PersistenceStore & store) override;
	void dummy() const override;
};

struct TestObject : public Persistence::Persistable {
	TestObject() = default;

	float flt {};
	std::string str {};
	bool bl {};
	glm::vec3 pos {};
	std::vector<float> flts;
	std::vector<glm::vec3> poss;
	std::vector<std::vector<std::vector<std::string>>> nest;
	std::unique_ptr<TestObject> ptr;
	std::unique_ptr<AbsObject> aptr;
	std::vector<std::unique_ptr<TestObject>> vptr;

	unsigned int postLoadCalled {};

	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};

struct SharedTestObject : public Persistence::Persistable {
	SharedTestObject() = default;

	std::shared_ptr<AbsObject> sptr;
	std::shared_ptr<SubObject> ssptr;

	bool persist(Persistence::PersistenceStore & store) override;
};
