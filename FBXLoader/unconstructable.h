#pragma once

#include "Uncopiable.hpp"

class Unconstructable : private Uncopiable
{
private:
	Unconstructable() = delete;
};