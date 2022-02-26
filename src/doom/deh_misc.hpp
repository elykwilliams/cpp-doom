//
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
//
// Parses "Misc" sections in dehacked files
//

#pragma once

constexpr auto DEH_DEFAULT_INITIAL_HEALTH     = 100;
constexpr auto DEH_DEFAULT_INITIAL_BULLETS    = 50;
constexpr auto DEH_DEFAULT_MAX_HEALTH         = 200;
constexpr auto DEH_DEFAULT_MAX_ARMOR          = 200;
constexpr auto DEH_DEFAULT_GREEN_ARMOR_CLASS  = 1;
constexpr auto DEH_DEFAULT_BLUE_ARMOR_CLASS   = 2;
constexpr auto DEH_DEFAULT_MAX_SOULSPHERE     = 200;
constexpr auto DEH_DEFAULT_SOULSPHERE_HEALTH  = 100;
constexpr auto DEH_DEFAULT_MEGASPHERE_HEALTH  = 200;
constexpr auto DEH_DEFAULT_GOD_MODE_HEALTH    = 100;
constexpr auto DEH_DEFAULT_IDFA_ARMOR         = 200;
constexpr auto DEH_DEFAULT_IDFA_ARMOR_CLASS   = 2;
constexpr auto DEH_DEFAULT_IDKFA_ARMOR        = 200;
constexpr auto DEH_DEFAULT_IDKFA_ARMOR_CLASS  = 2;
constexpr auto DEH_DEFAULT_BFG_CELLS_PER_SHOT = 40;
constexpr auto DEH_DEFAULT_SPECIES_INFIGHTING = 0;

extern int deh_initial_health;
extern int deh_initial_bullets;
extern int deh_max_health;
extern int deh_max_armor;
extern int deh_green_armor_class;
extern int deh_blue_armor_class;
extern int deh_max_soulsphere;
extern int deh_soulsphere_health;
extern int deh_megasphere_health;
extern int deh_god_mode_health;
extern int deh_idfa_armor;
extern int deh_idfa_armor_class;
extern int deh_idkfa_armor;
extern int deh_idkfa_armor_class;
extern int deh_bfg_cells_per_shot;
extern int deh_species_infighting;
