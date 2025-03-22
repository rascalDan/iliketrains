#pragma once

#include <memory>
#include <tuple>

template<typename Primary, typename... Others> class ManyPtr : Primary {
public:
	using element_type = typename Primary::element_type;

	template<typename... Params> ManyPtr(Params &&... params) : Primary {std::forward<Params>(params)...}
	{
		updatePtrs();
	}

	using Primary::operator->;
	using Primary::operator*;
	using Primary::operator bool;
	using Primary::get;

	template<typename... Params>
	void
	reset(Params &&... params)
	{
		Primary::reset(std::forward<Params>(params)...);
		updatePtrs();
	}

	template<typename Other>
	[[nodiscard]] consteval static bool
	couldBe()
	{
		return (std::is_convertible_v<Others *, Other *> || ...);
	}

	template<typename Other>
		requires(couldBe<Other>())
	[[nodiscard]] auto
	getAs() const
	{
		return std::get<idx<Other>()>(others);
	}

	template<typename Other>
		requires(!couldBe<Other>() && requires { std::dynamic_pointer_cast<Other>(std::declval<Primary>()); })
	[[nodiscard]] auto
	dynamicCast() const
	{
		return std::dynamic_pointer_cast<Other>(*this);
	}

	template<typename Other>
		requires(!couldBe<Other>() && !requires { std::dynamic_pointer_cast<Other>(std::declval<Primary>()); })
	[[nodiscard]] auto
	dynamicCast() const
	{
		return dynamic_cast<Other *>(get());
	}

private:
	using OtherPtrs = std::tuple<Others *...>;

	template<typename Other>
		requires(couldBe<Other>())
	[[nodiscard]] consteval static bool
	idx()
	{
		size_t typeIdx = 0;
		return ((typeIdx++ && std::is_convertible_v<Others *, Other *>) || ...);
	}

	void
	updatePtrs()
	{
		if (*this) {
			others = {dynamic_cast<Others *>(get())...};
		}
		else {
			others = {};
		}
	}

	OtherPtrs others;
};

template<typename Primary, typename... Others> using ManySharedPtr = ManyPtr<std::shared_ptr<Primary>, Others...>;
template<typename Primary, typename... Others> using ManyUniquePtr = ManyPtr<std::unique_ptr<Primary>, Others...>;
