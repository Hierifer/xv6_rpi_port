//
// synchronize.cpp
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


#include <types.h>
#include "synchronize.h"


#define SETWAY_LEVEL_SHIFT           1

#define L1_DATA_CACHE_SETS         128
#define L1_DATA_CACHE_WAYS           4
#define L1_SETWAY_WAY_SHIFT	        30  // 32-Log2(L1_DATA_CACHE_WAYS)
#define L1_DATA_CACHE_LINE_LENGTH	64
#define L1_SETWAY_SET_SHIFT	         6  // Log2(L1_DATA_CACHE_LINE_LENGTH)

#define L2_CACHE_SETS             1024
#define L2_CACHE_WAYS                8
#define L2_SETWAY_WAY_SHIFT         29  // 32-Log2(L2_CACHE_WAYS)
#define L2_CACHE_LINE_LENGTH        64
#define L2_SETWAY_SET_SHIFT          6  // Log2(L2_CACHE_LINE_LENGTH)

#define DATA_CACHE_LINE_LENGTH_MIN  64  // min(L1_DATA_CACHE_LINE_LENGTH, L2_CACHE_LINE_LENGTH)

void InvalidateDataCache (void)
{
	// invalidate L1 data cache
        register unsigned nSet;
	for (nSet = 0; nSet < L1_DATA_CACHE_SETS; nSet++)
	{
                register unsigned nWay;
		for (nWay = 0; nWay < L1_DATA_CACHE_WAYS; nWay++)
		{
			register u32 nSetWayLevel =   nWay << L1_SETWAY_WAY_SHIFT
						    | nSet << L1_SETWAY_SET_SHIFT
						    | 0 << SETWAY_LEVEL_SHIFT;

			__asm volatile ("mcr p15, 0, %0, c7, c6,  2" : : "r" (nSetWayLevel) : "memory");	// DCISW
		}
	}

	// invalidate L2 unified cache
	for (nSet = 0; nSet < L2_CACHE_SETS; nSet++)
	{
                register unsigned nWay;
		for (nWay = 0; nWay < L2_CACHE_WAYS; nWay++)
		{
			register u32 nSetWayLevel =   nWay << L2_SETWAY_WAY_SHIFT
						    | nSet << L2_SETWAY_SET_SHIFT
						    | 1 << SETWAY_LEVEL_SHIFT;

			__asm volatile ("mcr p15, 0, %0, c7, c6,  2" : : "r" (nSetWayLevel) : "memory");	// DCISW
		}
	}
}

void CleanDataCache (void)
{
	// clean L1 data cache
        register unsigned nSet;
	for (nSet = 0; nSet < L1_DATA_CACHE_SETS; nSet++)
	{
                register unsigned nWay;
		for (nWay = 0; nWay < L1_DATA_CACHE_WAYS; nWay++)
		{
			register u32 nSetWayLevel =   nWay << L1_SETWAY_WAY_SHIFT
						    | nSet << L1_SETWAY_SET_SHIFT
						    | 0 << SETWAY_LEVEL_SHIFT;

			__asm volatile ("mcr p15, 0, %0, c7, c10,  2" : : "r" (nSetWayLevel) : "memory");	// DCCSW
		}
	}

	// clean L2 unified cache
	for (nSet = 0; nSet < L2_CACHE_SETS; nSet++)
	{
                register unsigned nWay;
		for (nWay = 0; nWay < L2_CACHE_WAYS; nWay++)
		{
			register u32 nSetWayLevel =   nWay << L2_SETWAY_WAY_SHIFT
						    | nSet << L2_SETWAY_SET_SHIFT
						    | 1 << SETWAY_LEVEL_SHIFT;

			__asm volatile ("mcr p15, 0, %0, c7, c10,  2" : : "r" (nSetWayLevel) : "memory");	// DCCSW
		}
	}
}

void InvalidateDataCacheRange (u32 nAddress, u32 nLength)
{
	nLength += DATA_CACHE_LINE_LENGTH_MIN;

	while (1)
	{
		__asm volatile ("mcr p15, 0, %0, c7, c6,  1" : : "r" (nAddress) : "memory");	// DCIMVAC

		if (nLength < DATA_CACHE_LINE_LENGTH_MIN)
		{
			break;
		}

		nAddress += DATA_CACHE_LINE_LENGTH_MIN;
		nLength  -= DATA_CACHE_LINE_LENGTH_MIN;
	}
}

void CleanDataCacheRange (u32 nAddress, u32 nLength)
{
	nLength += DATA_CACHE_LINE_LENGTH_MIN;

	while (1)
	{
		__asm volatile ("mcr p15, 0, %0, c7, c10,  1" : : "r" (nAddress) : "memory");	// DCCMVAC

		if (nLength < DATA_CACHE_LINE_LENGTH_MIN)
		{
			break;
		}

		nAddress += DATA_CACHE_LINE_LENGTH_MIN;
		nLength  -= DATA_CACHE_LINE_LENGTH_MIN;
	}
}

void CleanAndInvalidateDataCacheRange (u32 nAddress, u32 nLength)
{
	nLength += DATA_CACHE_LINE_LENGTH_MIN;

	while (1)
	{
		__asm volatile ("mcr p15, 0, %0, c7, c14,  1" : : "r" (nAddress) : "memory");	// DCCIMVAC

		if (nLength < DATA_CACHE_LINE_LENGTH_MIN)
		{
			break;
		}

		nAddress += DATA_CACHE_LINE_LENGTH_MIN;
		nLength  -= DATA_CACHE_LINE_LENGTH_MIN;
	}
}

