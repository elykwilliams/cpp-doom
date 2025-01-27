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
//

#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include <cstdarg>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// intentional newline to avoid reordering by Clang Format
#else
#include <unistd.h>
#endif

#include <SDL.h>

#include "doomtype.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp"

#include "i_system.hpp"

#include "memory.hpp"
#include "z_zone.hpp"

constexpr auto DEFAULT_RAM = 16 * 2; /* MiB [crispy] */
constexpr auto MIN_RAM     = 4 * 4;  /* MiB [crispy] */

using atexit_listentry_t = struct atexit_listentry_s;

struct atexit_listentry_s {
  atexit_func_t        func;
  bool                 run_on_error;
  atexit_listentry_t * next;
};

static atexit_listentry_t * exit_funcs = nullptr;

void I_AtExit(atexit_func_t func, bool run_on_error) {
  auto * entry = create_struct<atexit_listentry_t>();

  entry->func         = func;
  entry->run_on_error = run_on_error;
  entry->next         = exit_funcs;
  exit_funcs          = entry;
}

// Tactile feedback function, probably used for the Logitech Cyberman

void I_Tactile(int, int, int) {
}

// Zone memory auto-allocation function that allocates the zone size
// by trying progressively smaller zone sizes until one is found that
// works.

static uint8_t * AutoAllocMemory(int * size, int default_ram, int min_ram) {
  // Allocate the zone memory.  This loop tries progressively smaller
  // zone sizes until a size is found that can be allocated.
  // If we used the -mb command line parameter, only the parameter
  // provided is accepted.

  uint8_t * zonemem = nullptr;

  while (zonemem == nullptr) {
    // We need a reasonable minimum amount of RAM to start.

    if (default_ram < min_ram) {
      I_Error("Unable to allocate %i MiB of RAM for zone", default_ram);
    }

    // Try to allocate the zone memory.

    *size = default_ram * 1024 * 1024;

    zonemem = static_cast<uint8_t *>(malloc(static_cast<size_t>(*size)));

    // Failed to allocate?  Reduce zone size until we reach a size
    // that is acceptable.

    if (zonemem == nullptr) {
      default_ram -= 1;
    }
  }

  return zonemem;
}

uint8_t * I_ZoneBase(int * size) {
  static int i = 1;

  //!
  // @category obscure
  // @arg <mb>
  //
  // Specify the heap size, in MiB (default 16).
  //

  int p           = M_CheckParmWithArgs("-mb", 1);
  int min_ram     = MIN_RAM;
  int default_ram = DEFAULT_RAM;

  if (p > 0) {
    default_ram = std::atoi(myargv[p + 1]);
    min_ram     = default_ram;
  }

  // [crispy] do not allocate new zones ad infinitum
  if (i > 8) {
    min_ram = default_ram + 1;
  }

  uint8_t * zonemem = AutoAllocMemory(size, default_ram * i, min_ram * i);

  // [crispy] if called again, allocate another zone twice as big
  i *= 2;

  fmt::printf("zone memory: %p, %d MiB allocated for zone\n",
                fmt::ptr(zonemem),
                *size >> 20); // [crispy] human-understandable zone heap size

  return zonemem;
}

void I_PrintBanner(cstring_view msg) {
  size_t spaces = 35 - (msg.size() / 2);

  for (size_t i = 0; i < spaces; ++i)
    putchar(' ');

  puts(msg.c_str());
}

void I_PrintDivider() {
  for (int i = 0; i < 75; ++i) {
    putchar('=');
  }

  putchar('\n');
}

void I_PrintStartupBanner(cstring_view gamedescription) {
  I_PrintDivider();
  I_PrintBanner(gamedescription);
  I_PrintDivider();

  fmt::printf(
      " " PACKAGE_NAME " is free software, covered by the GNU General Public\n"
      " License.  There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
      " FOR A PARTICULAR PURPOSE. You are welcome to change and distribute\n"
      " copies under certain conditions. See the source for more information.\n");

  I_PrintDivider();
}

//
// I_ConsoleStdout
//
// Returns true if stdout is a real console, false if it is a file
//

bool I_ConsoleStdout() {
#ifdef _WIN32
  // SDL "helpfully" always redirects stdout to a file.
  return false;
#else
  return isatty(fileno(stdout));
#endif
}

//
// I_Init
//
/*
void I_Init ()
{
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
}
void I_BindVariables()
{
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();
}
*/

//
// I_Quit
//

void I_Quit() {
  // Run through all exit functions

  atexit_listentry_t * entry = exit_funcs;

  while (entry != nullptr) {
    entry->func();
    entry = entry->next;
  }

  SDL_Quit();

  exit(0);
}

//
// I_Error
//

static bool already_quitting = false;

void I_Error(const char * error, ...) {
  char                 msgbuf[512];
  va_list              argptr;

  if (already_quitting) {
    fmt::fprintf(stderr, "Warning: recursive call to I_Error detected.\n");
    exit(-1);
  } else {
    already_quitting = true;
  }

  // Message first.
  va_start(argptr, error);
  // fprintf(stderr, "\nError: ");
  vfprintf(stderr, error, argptr);
  fmt::fprintf(stderr, "\n\n");
  va_end(argptr);
  fflush(stderr);

  // Write a copy of the message into buffer.
  va_start(argptr, error);
  std::memset(msgbuf, 0, sizeof(msgbuf));
  M_vsnprintf(msgbuf, sizeof(msgbuf), error, argptr);
  va_end(argptr);

  // Shutdown. Here might be other errors.

  atexit_listentry_t * entry = exit_funcs;

  while (entry != nullptr) {
    if (entry->run_on_error) {
      entry->func();
    }

    entry = entry->next;
  }

  //!
  // @category obscure
  //
  // If specified, don't show a GUI window for error messages when the
  // game exits with an error.
  //
  bool exit_gui_popup = !M_ParmExists("-nogui");

  // Pop up a GUI dialog box to show the error message, if the
  // game was not run from the console (and the user will
  // therefore be unable to otherwise see the message).
  if (exit_gui_popup && !I_ConsoleStdout()) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                             PACKAGE_STRING,
                             msgbuf,
                             nullptr);
  }

  // abort();

  SDL_Quit();

  exit(-1);
}

//
// I_Realloc
//

void * I_Realloc(void * ptr, size_t size) {
  void * new_ptr = realloc(ptr, size);

  if (size != 0 && new_ptr == nullptr) {
    I_Error("I_Realloc: failed on reallocation of %" PRIuPTR " bytes", size);
  }

  return new_ptr;
}

//
// Read Access Violation emulation.
//
// From PrBoom+, by entryway.
//

// C:\>debug
// -d 0:0
//
// DOS 6.22:
// 0000:0000  (57 92 19 00) F4 06 70 00-(16 00)
// DOS 7.1:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// Win98:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// DOSBox under XP:
// 0000:0000  (00 00 00 F1) ?? ?? ?? 00-(07 00)

constexpr auto DOS_MEM_DUMP_SIZE = 10;

static const unsigned char mem_dump_dos622[DOS_MEM_DUMP_SIZE] = {
  0x57,
  0x92,
  0x19,
  0x00,
  0xF4,
  0x06,
  0x70,
  0x00,
  0x16,
  0x00
};
static const unsigned char mem_dump_win98[DOS_MEM_DUMP_SIZE] = {
  0x9E,
  0x0F,
  0xC9,
  0x00,
  0x65,
  0x04,
  0x70,
  0x00,
  0x16,
  0x00
};
static const unsigned char mem_dump_dosbox[DOS_MEM_DUMP_SIZE] = {
  0x00,
  0x00,
  0x00,
  0xF1,
  0x00,
  0x00,
  0x00,
  0x00,
  0x07,
  0x00
};
static unsigned char mem_dump_custom[DOS_MEM_DUMP_SIZE];

static const unsigned char * dos_mem_dump = mem_dump_dos622;

bool I_GetMemoryValue(unsigned int offset, void * value, int size) {
  static bool firsttime = true;

  if (firsttime) {
    firsttime = false;

    //!
    // @category compat
    // @arg <version>
    //
    // Specify DOS version to emulate for nullptr pointer dereference
    // emulation.  Supported versions are: dos622, dos71, dosbox.
    // The default is to emulate DOS 7.1 (Windows 98).
    //

    int p = M_CheckParmWithArgs("-setmem", 1);

    if (p > 0) {
      if (iequals(myargv[p + 1], "dos622")) {
        dos_mem_dump = mem_dump_dos622;
      }
      if (iequals(myargv[p + 1], "dos71")) {
        dos_mem_dump = mem_dump_win98;
      } else if (iequals(myargv[p + 1], "dosbox")) {
        dos_mem_dump = mem_dump_dosbox;
      } else {
        for (int i = 0; i < DOS_MEM_DUMP_SIZE; ++i) {
          ++p;

          if (p >= myargc || myargv[p][0] == '-') {
            break;
          }

          int val = 0;
          M_StrToInt(myargv[p], &val);
          mem_dump_custom[i++] = static_cast<unsigned char>(val);
        }

        dos_mem_dump = mem_dump_custom;
      }
    }
  }

  switch (size) {
  case 1:
    *(reinterpret_cast<unsigned int *>(value)) = dos_mem_dump[offset];
    return true;
  case 2:
    *(reinterpret_cast<unsigned int *>(value)) = static_cast<unsigned int>(dos_mem_dump[offset] | (dos_mem_dump[offset + 1] << 8));
    return true;
  case 4:
    *(reinterpret_cast<unsigned int *>(value)) = static_cast<unsigned int>(
        dos_mem_dump[offset] | (dos_mem_dump[offset + 1] << 8) | (dos_mem_dump[offset + 2] << 16) | (dos_mem_dump[offset + 3] << 24));
    return true;
  }

  return false;
}
