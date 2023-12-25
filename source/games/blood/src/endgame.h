/*
 * Copyright (C) 2023 Christoph Oelckers
 *
 * This file is part of Raze.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
//-------------------------------------------------------------------------
#pragma once
#include "build.h"
#include "common_game.h"

BEGIN_BLD_NS

bool AllowedKillType(DBloodActor* actor);
void AddKill(DBloodActor* killer, DBloodActor* killed);


END_BLD_NS
