/*! \file cxlib.h
	\brief Single header file for entire library
*/

/*! \mainpage Overview
 C Extensions, or CX for short, is a collection of extensions to the C programing language, packaged as a single shared library. These extensions include exception handling, automatic dynamic memory deallocation and destruction, a universal object interface, and generic containers. The library is written in C and is not hardware dependent. Currently only POSIX platforms are supported but a Windows port is planned. Please note that this project is still in the early stages of development.

*/

#if !CXLIB_VERSION

	#define CXLIB_VERSION 1000000L

	#include <cxrt.h>

#endif
