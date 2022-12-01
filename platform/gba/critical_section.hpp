#pragma once

#include "number/int.hpp"
#include <tuple>


namespace flash_filesystem
{


using IrqState = std::pair<u16, u16>;
IrqState critical_section_enter();
void critical_section_exit(IrqState state);



}
