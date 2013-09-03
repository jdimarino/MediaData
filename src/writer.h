/*
 * writer.h
 * Header file of the Writer for thetvdb.com's web API modified for 
 * generating XML documents formatted for the WDTV.
 *
 * Copyright (C) 2013 - Joseph DiMarino
 *
 * This file is part of MediaData.
 *
 * MediaData is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MediaData is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MediaData.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef WRITER_H
#define WRITER_H
#include "parser.h"

MYBOOL write_episode(episode *episode_to_write,
                      series_information *series_info_to_write, char* path);

#endif
