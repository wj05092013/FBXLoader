#pragma once

namespace ba
{
	namespace util
	{
#define ReleaseCOM(o) { if (o) { (o)->Release(); (o) = nullptr; } }

		
	}
}