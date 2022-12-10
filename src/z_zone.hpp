//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//	Remark: this was the only stuff that, according
//	 to John Carmack, might have been useful for
//	 Quake.
//

#pragma once

#include <cstdio>
#include <source_location>

#include "cstring_view.hpp"

//
// ZONE MEMORY
// PU - purge tags.

enum
{
  PU_STATIC = 1,             // static entire execution time
  PU_SOUND [[maybe_unused]], // static while playing
  PU_MUSIC [[maybe_unused]], // static while playing
  PU_FREE,                   // a free block
  PU_LEVEL,                  // static until level exited
  PU_LEVSPEC,                // a special thinker in a level

  // Tags >= PU_PURGELEVEL are purgable whenever needed.

  PU_PURGELEVEL,
  PU_CACHE,

  // Total number of different tag types

  PU_NUM_TAGS
};

void                          Z_Init();
void *                        Z_Malloc(std::size_t size, int tag, void * user);
void                          Z_Free(void * ptr);
void                          Z_FreeTags(int lowtag, int hightag);
[[maybe_unused]] void         Z_DumpHeap(int lowtag, int hightag);
[[maybe_unused]] void         Z_FileDumpHeap(FILE * f);
void                          Z_CheckHeap();
void                          Z_ChangeTag(void * ptr, int tag, std::source_location src = std::source_location::current());
[[maybe_unused]] void         Z_ChangeUser(void * ptr, void ** user);
[[maybe_unused]] std::size_t  Z_FreeMemory();
[[maybe_unused]] std::size_t  Z_ZoneSize();

