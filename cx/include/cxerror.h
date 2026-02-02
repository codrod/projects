/*! \file cxerror.h
	\brief Error codes shared by the entire library are defined here
*/

#if !CXERROR_VERSION

	#define CXERROR_VERSION 1000000L

	#define CXERROR_MAIN -2018
	#define CXERROR_FUNC -2019
	#define CXERROR_THREAD -2020
	#define CXERROR_RETURN -2021

	#define CXERROR_THROW -2022
	#define CXERROR_TRY -2023

	#define CXERROR_NOMEM -2023
	#define CXERROR_EMPTY -2024
	#define CXERROR_ARG -2025
	#define CXERROR_SIGNAL -2026
	#define CXERROR_NOT_FOUND -2027
	#define CXERROR_ADDRESS -2028
	#define CXERROR_SIZE -2029
	#define CXERROR_PUSH -2030

	/*
	#define CX_ERR_COP -7/*!
	#define CX_ERR_CON -8
	#define CX_ERR_DES -9
	#define CX_ERR_OVE -10
	#define CX_ERR_OUT -11
	#define CX_ERR_NUL -12
	#define CX_ERR_SIZ -13
	#define CX_ERR_STR -14
	*/

#endif
