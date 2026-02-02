#include "gtb.h"

/*
*/

using namespace GTB;

Sequence::Sequence(Glib::ustring str)
{
	try
	{
		parse(str);
	}
	catch(...)
	{
		arg.clear();
	}

	return;
}

Sequence::Sequence()
{
	return;
}

Sequence& Sequence::parse(Glib::ustring str)
{
	Glib::ustring element;
	size_t i = 1; //skip DC1

	if(!str.size()) return *this;

	if(*str.begin() != DC1)
		throw Error("string does not start with DC1");

	if(*str.rbegin() != DC3)
		throw Error("string does not end with DC3");

	for(; i + 1 < str.size(); i++)
	{
		if(str[i] == DC1 || str[i] == DC3)
			throw Error("found DC1 or DC3 in middle of string");

		if(str[i] == DC2)
		{
			if(!element.size())
				throw Error("found empty argument in string");

			arg.push_back(element);
			element.clear();
		}
		else element.push_back(str[i]);
	}

	if(!element.size())
		throw Error("found empty argument in string");

	arg.push_back(element);

	return *this;
}
