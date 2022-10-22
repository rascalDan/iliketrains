#pragma once

#include <memory>
union SDL_Event;

enum ModeSecondClick { Unset, Reset, NoAction };
template<typename Target, ModeSecondClick msc = ModeSecondClick::Unset> class Mode {
public:
	explicit Mode(Target & t) : target {t} { }

	Target & target;

	template<typename Mode, typename... Params>
	auto
	toggle(Params &&... params)
	{
		return [params..., this](const SDL_Event &) {
			toggleSetMode<Mode>(std::forward<Params>(params)...);
		};
	}

private:
	template<typename Mode, typename... Params>
	void
	toggleSetMode(Params &&... params)
	{
		if (dynamic_cast<Mode *>(target.get())) {
			if constexpr (msc == ModeSecondClick::Unset) {
				target.reset();
			}
			if constexpr (msc == ModeSecondClick::Reset) {
				target = std::make_unique<Mode>(std::forward<Params>(params)...);
			}
		}
		else {
			target = std::make_unique<Mode>(std::forward<Params>(params)...);
		}
	}
};
