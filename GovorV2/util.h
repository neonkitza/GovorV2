
#include <string>

#pragma once
using namespace std;

bool nameBeginsWith(string s, string t)
{
	if (s.compare(0, t.length(), t) == 0)
	{
		return true;
	}
	return false;
}