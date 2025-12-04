/*!
*
* \file
* \brief GPIO.cpp header file
*
===============================================================================

 Name        : DEBUG.h

 Authors     : Marc Obbad

 Version     :

 Description : DEBUG_SOS interface functions

===============================================================================

Copyright   : (C) Copyright 2025 Empower RF Systems

===============================================================================
*
*
*
*
*  Created on: OCT 2025
*      Author: Marc Obbad
*
*
*/
#pragma once
#ifdef DEBUG
#include <cstdio>
#define ENTER   printf("\n=======  Entering function %s =======\n", __func__);
#define EXIT    printf("\n=======  Exiting  function %s =======\n", __func__);
#define dbgprintf printf
#else
#define ENTER   
#define EXIT   
#define dbgprintf(...) (void)0  // Safe no-op for variadic macro
#endif
