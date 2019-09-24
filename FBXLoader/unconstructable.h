#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Unconstructable : private Uncopiable
	{
	private:
		Unconstructable() = delete;
	};
}
