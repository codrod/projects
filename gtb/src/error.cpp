#include "gtb.h"

using namespace GTB;

Error::Error(const std::string& str)
{
	msg = str;

	if(errno)
	{
		msg += " : ";
		msg += strerror(errno);
	}

	return;
}

Error::Error(const char *str)
{
	msg = str;

	if(errno)
	{
		msg += " : ";
		msg += strerror(errno);
	}

	return;
}

const char* Error::what() const noexcept
{
	return msg.data();
}

FatalError::FatalError(const std::string& str)
{
	msg = str;

	if(errno)
	{
		msg += " : ";
		msg += strerror(errno);
	}

	return;
}

FatalError::FatalError(const char *str)
{
	msg = str;

	if(errno)
	{
		msg += " : ";
		msg += strerror(errno);
	}

	return;
}

const char* FatalError::what() const noexcept
{
	return msg.data();
}
