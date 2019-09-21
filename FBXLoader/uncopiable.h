#pragma once

class Uncopiable
{
public:
	Uncopiable() {}

private:
	Uncopiable(const Uncopiable&) = delete;
	Uncopiable& operator=(const Uncopiable&) = delete;
};