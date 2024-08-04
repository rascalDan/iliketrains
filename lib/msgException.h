#pragma once

#include <optional>
#include <string>

template<typename BaseException> class MsgException : public BaseException {
protected:
	using BaseException::BaseException;
	[[nodiscard]] virtual std::string getMsg() const noexcept = 0;

public:
	[[nodiscard]] const char *
	what() const noexcept override
	{
		if (!msg) {
			msg.emplace(getMsg());
		}
		return msg->c_str();
	}

private:
	mutable std::optional<std::string> msg;
};
