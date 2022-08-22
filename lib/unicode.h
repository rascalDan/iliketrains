#pragma once

// Wrappers of some glib functions (why are we using glib then?) which we want, but glib.h is a bit C like

#ifdef __cplusplus
#	include <cstdint>
extern "C" {
#else
#	include <stdint.h>
#endif

const char * next_char(const char *);
uint32_t get_codepoint(const char *);

#ifdef __cplusplus
}

#	include <string_view>
#	include <algorithm>

struct utf8_string_view {
	struct iter {
		constexpr explicit iter(const char * p) : pos {p} { }

		[[nodiscard]] auto
		operator!=(const iter & other) const
		{
			return pos != other.pos;
		}

		auto &
		operator++()
		{
			pos = next_char(pos);
			return *this;
		}

		[[nodiscard]] auto
		operator*() const
		{
			return get_codepoint(pos);
		}

	private:
		const char * pos;
	};

	template<typename Str>
	// cppcheck-suppress noExplicitConstructor; NOLINTNEXTLINE(hicpp-explicit-conversions)
	constexpr utf8_string_view(const Str & str) : begin_ {str.data()}, end_ {str.data() + str.length()}
	{
	}
	// cppcheck-suppress noExplicitConstructor; NOLINTNEXTLINE(hicpp-explicit-conversions)
	constexpr utf8_string_view(const char * const str) : utf8_string_view {std::string_view {str}} { }

	[[nodiscard]] auto
	begin() const
	{
		return iter {begin_};
	}

	[[nodiscard]] auto
	end() const
	{
		return iter {end_};
	}

	[[nodiscard]] size_t length() const;

private:
	const char *begin_, *end_;
};
template<> struct std::iterator_traits<utf8_string_view::iter> {
	using difference_type = size_t;
	using value_type = uint32_t;
	using pointer = void;
	using reference = void;
	using iterator_category = std::forward_iterator_tag;
};

[[nodiscard]] inline size_t
utf8_string_view::length() const
{
	return std::distance(begin(), end());
}

#endif
