#pragma once

#define ReleaseCOM(o) { if (o) { (o)->Release(); (o) = nullptr; } }
