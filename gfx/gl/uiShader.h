#ifndef UISHADER_H
#define UISHADER_H

#include "programHandle.h"
#include <cstdint>

class UIShader {
public:
	UIShader(std::size_t width, std::size_t height);

private:
	class UIProgramHandle : public ProgramHandleBase {
		using ProgramHandleBase::ProgramHandleBase;
	};
	UIProgramHandle program;
};

#endif
