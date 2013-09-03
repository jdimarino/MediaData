/*
 * parser.h
 * Header file for the thetvdb.com's web API parser.
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

#ifndef PARSER_H
#define PARSER_H


typedef struct {
    char *episode_name;
    char *episode_number;
    char *season_number;
    char *firstaired;
    char *overview;
    char **directors;
    char **actors;
    char *backdrop;
} episode;

typedef struct {
    char *series_ID;
    char *series_name;
    char **genre;
    char *mpaa;
    char *runtime;
    char **backdrops;
    char ***season_banners;
    char ***wide_season_banners;
    char **season_number_index;
    char **wide_season_number_index;
    int num_season_banners;
    int num_wide_season_banners;
} series_information;

typedef struct {
    episode **episodes;
    int num_episodes;
} season;

typedef struct {
    season **seasons;
    int num_seasons;
    series_information *series_info;
} series;

typedef enum { False, True } MYBOOL;

void download_url(char *url, char *loc);

void free_series(series* series_to_free);

MYBOOL retrieve_series_backdrops(series_information *series_info);

series_information* retrieve_series_info(char *series_ID);

episode* retrieve_episode(char* series_ID, int s_num, int e_num, 
                          MYBOOL dvd_order);

series* parse(char* series_ID, char* lang, MYBOOL dvd_order);

#endif
