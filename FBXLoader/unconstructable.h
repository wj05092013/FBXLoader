#pragma once

namespace ba
{
	class Unconstructable : private Uncopiable
	{
	private:
		Unconstructable() = delete;
	};
}
