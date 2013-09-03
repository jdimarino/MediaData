/*
 * parser.c
 * Parser for thetvdb.com's web API.
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

#if WIN32
    #define _CRT_SECURE_NO_DEPRECATE 1
    #define snprintf sprintf_s
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "parser.h"
#include <curl/curl.h>
#include <curl/easy.h>


#ifdef LIBXML_TREE_ENABLED

static const int MAX_NUM_ACTORS = 100;
static const int MAX_NUM_DIRECTORS = 100;
static const int MAX_NUM_GENRES = 100;
static const int MAX_URL_LEN = 100;
static const int MAX_NUM_EPS = 1000;
static const int MAX_NUM_SEASONS = 3000;
static const int MAX_NUM_BAN = 1000;
static MYBOOL invalid_order_type = False;

void free_series_information(series_information *series_info_to_free) 
{
    int j, i;
    
    xmlFree(series_info_to_free->series_ID);
    xmlFree(series_info_to_free->series_name);
    xmlFree(series_info_to_free->genre[MAX_NUM_GENRES-1]);
    free(series_info_to_free->genre);
    xmlFree(series_info_to_free->mpaa);
    xmlFree(series_info_to_free->runtime);
    j = 0;
    while (series_info_to_free->backdrops[j]){
        free(series_info_to_free->backdrops[j]);
        j++;
    }
    free(series_info_to_free->backdrops);
    j = 0;
    while (series_info_to_free->season_banners[j]){
        i = 0;
        while (series_info_to_free->season_banners[j][i]) {
            free(series_info_to_free->season_banners[j][i]);
            i++;
        }
        free(series_info_to_free->season_banners[j]);
        j++;
    }
    free(series_info_to_free->season_banners);
    j = 0;
    while (series_info_to_free->wide_season_banners[j]){
        i = 0;
        while (series_info_to_free->wide_season_banners[j][i]) {
            free(series_info_to_free->wide_season_banners[j][i]);
            i++;
        }
        free(series_info_to_free->wide_season_banners[j]);
        j++;
    }
    free(series_info_to_free->wide_season_banners);
    
    j = 0;
    while (series_info_to_free->season_number_index[j]){
        free(series_info_to_free->season_number_index[j]);
        j++;
    }
    free(series_info_to_free->season_number_index);

    j = 0;
    while (series_info_to_free->wide_season_number_index[j]){
        free(series_info_to_free->wide_season_number_index[j]);
        j++;
    }
    free(series_info_to_free->wide_season_number_index);
    
    free(series_info_to_free);
}

episode* init_episode() {
    episode* new_episode = (episode*)malloc(sizeof(episode));
    new_episode->episode_name = NULL;
    new_episode->episode_number = NULL;
    new_episode->season_number = NULL;
    new_episode->firstaired = NULL;
    new_episode->actors = NULL;
    new_episode->backdrop = NULL;
    new_episode->overview = NULL;
    new_episode->directors = NULL;
    return new_episode;
    
}

void free_episode(episode* episode_to_free)
{
    if (episode_to_free->episode_name)
        xmlFree(episode_to_free->episode_name);

    if (episode_to_free->episode_number)
        xmlFree(episode_to_free->episode_number);
    if (episode_to_free->season_number)
        xmlFree(episode_to_free->season_number);
    if (episode_to_free->firstaired)
        xmlFree(episode_to_free->firstaired);

    if (episode_to_free->actors) {
        if (episode_to_free->actors[MAX_NUM_ACTORS-1])
            xmlFree(episode_to_free->actors[MAX_NUM_ACTORS-1]);
        free(episode_to_free->actors);
    }
    
    if (episode_to_free->backdrop)
        free(episode_to_free->backdrop);
    if (episode_to_free->overview)
        xmlFree(episode_to_free->overview);

    if (episode_to_free->directors) {
        if (episode_to_free->directors[MAX_NUM_DIRECTORS-1])
            xmlFree(episode_to_free->directors[MAX_NUM_DIRECTORS-1]);
        free(episode_to_free->directors);
    }
    
    if (episode_to_free) {
        free(episode_to_free);
    }
}

void in_place_adjust_season(season* season_to_adjust) {
    int num_eps = season_to_adjust->num_episodes;
    int complete = 0; int pos = 0; int runner = 0;
    
    while (complete < num_eps) {
        if (season_to_adjust->episodes[pos++]) {
            complete++;
            runner = pos;
        } else {
            while(!(season_to_adjust->episodes[++runner]));
            season_to_adjust->episodes[pos-1] = 
                season_to_adjust->episodes[runner];
            season_to_adjust->episodes[runner] = NULL;
            complete++;
        }
    }
}

void in_place_adjust_series(series* series_to_adjust) {
    int num_seasons = series_to_adjust->num_seasons;
    int complete = 0; int pos = 0; int runner = 0;
    while (complete < num_seasons) {
        if (series_to_adjust->seasons[pos++]) {
            complete++;
            runner = pos;
        } else {
            while(!(series_to_adjust->seasons[++runner]));
            series_to_adjust->seasons[pos-1] = 
                series_to_adjust->seasons[runner];
            series_to_adjust->seasons[runner] = NULL;
            complete++;
        }
    }
}

void in_place_adjust_series_info(series_information *series_info_to_adjust) {
    int num_season_banners = series_info_to_adjust->num_season_banners;
    int num_wide_season_banners = 
        series_info_to_adjust->num_wide_season_banners;
    int complete = 0; int pos = 0; int runner = 0;
    while (complete < num_season_banners) {
        if (series_info_to_adjust->season_banners[pos++]) {
            complete++;
            runner = pos;
        } else {
            
            while(!(series_info_to_adjust->season_banners[++runner]));
            series_info_to_adjust->season_banners[pos-1] = 
                series_info_to_adjust->season_banners[runner];
            series_info_to_adjust->season_banners[runner] = NULL;
            complete++;
        }
    }
    complete = 0; pos = 0; runner = 0;
    while (complete < num_wide_season_banners) {
        if (series_info_to_adjust->wide_season_banners[pos++]) {
            complete++;
            runner = pos;
            
        } else {
            while(!(series_info_to_adjust->wide_season_banners[++runner]));
            series_info_to_adjust->wide_season_banners[pos-1] = 
                series_info_to_adjust->wide_season_banners[runner];
            series_info_to_adjust->wide_season_banners[runner] = NULL;
            complete++;
        }
    }
    
    complete = 0; pos = 0; runner = 0;
    while (complete < num_season_banners) {
        if (series_info_to_adjust->season_number_index[pos++]) {
            complete++;
            runner = pos;
        } else {
            while(!(series_info_to_adjust->season_number_index[++runner]));
            series_info_to_adjust->season_number_index[pos-1] = 
                series_info_to_adjust->season_number_index[runner];
            series_info_to_adjust->season_number_index[runner] = NULL;
            complete++;
        }
    }
    
    complete = 0; pos = 0; runner = 0;
    while (complete < num_wide_season_banners) {
        if (series_info_to_adjust->wide_season_number_index[pos++]) {
            complete++;
            runner = pos;
        } else {
            while(!(series_info_to_adjust->wide_season_number_index[++runner]));
            series_info_to_adjust->wide_season_number_index[pos-1] = 
                series_info_to_adjust->wide_season_number_index[runner];
            series_info_to_adjust->wide_season_number_index[runner] = NULL;
            complete++;
        }
    }
}

void free_season(season* season_to_free)
{
    int i;

    /* Free Episodes */
    for (i = 0; i < season_to_free->num_episodes; i++) {
        if (season_to_free->episodes[i]){
            free_episode(season_to_free->episodes[i]);
        }

    }
    free(season_to_free->episodes);
    free(season_to_free);
}

void free_series(series* series_to_free)
{
    int i;

    /* Free Seasons */
    for (i = 0; i < series_to_free->num_seasons; i++) {
        free_season(series_to_free->seasons[i]);
    }
    free(series_to_free->seasons);
    
    if (series_to_free->series_info) {
        free_series_information(series_to_free->series_info);
    }
    
    free(series_to_free);
}
 
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;
    
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void download_url(char *url, char *loc) 
{
    CURL *curl;
    FILE *fp;
    char *outfilename;
    
    
    char *subbuff = (char*)calloc(8,sizeof(char));
    char *real_loc;
    memcpy( subbuff, &loc[0], 7 );
    subbuff[7] = '\0';

    if (!(strcmp(subbuff, "file://"))) {
        real_loc = &loc[7];
        #if defined(__MINGW32__) || defined(WIN32)
        real_loc = &loc[8];
        #endif
    } else {
        real_loc = loc;
    }
    
    outfilename = real_loc;
    printf("loc: %s\n", outfilename);
    curl = curl_easy_init();
    
    if (curl) {
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    
    free(subbuff);
}

/**
* Retrieve a list of banner urls for a given series
**/
MYBOOL retrieve_series_backdrops(series_information *series_info)
{
    char **backdrops = (char **)malloc(MAX_NUM_BAN*sizeof(char*));
    char ***season_banners = (char ***)malloc(MAX_NUM_SEASONS*sizeof(char**));
    char ***wide_season_banners = 
        (char ***)malloc(MAX_NUM_SEASONS*sizeof(char**));
    char **season_number_index = (char**)malloc(MAX_NUM_SEASONS*sizeof(char*));
    char **wide_season_number_index = 
        (char**)malloc(MAX_NUM_SEASONS*sizeof(char*));
    xmlNode *root_element = NULL; xmlNode *cur_ban_node = NULL; 
    xmlNode *cur_node = NULL;
    xmlDoc *doc = NULL;
    int ban_index = 0;
    int i, j, ban_elems; 
    char *season_num;
    int num_season_banners = 0; int num_wide_season_banners = 0;
    char *url = (char*)malloc(MAX_URL_LEN*sizeof(char));
    char *path, *type;
    MYBOOL is_fan_art;
    MYBOOL is_season_ban;
    MYBOOL is_wide_season_ban;
    int season_banner_num = -1;
    char *series_ID = series_info->series_ID;
    for (j = 0; j < MAX_NUM_BAN; j++) {
        backdrops[j] = NULL;
    }
    for (j = 0; j < MAX_NUM_SEASONS; j++) {
        season_banners[j] = NULL;
    }
    for (j = 0; j < MAX_NUM_SEASONS; j++) {
        wide_season_banners[j] = NULL;
    }
    for (j = 0; j < MAX_NUM_SEASONS; j++) {
        season_number_index[j] = NULL;
    }
    for (j = 0; j < MAX_NUM_SEASONS; j++) {
        wide_season_number_index[j] = NULL;
    }
    
    sprintf(url, "http://thetvdb.com/data/series/%s/banners.xml", series_ID);
    /* parse the file and get the first DOM */
    doc = xmlReadFile(url, NULL, XML_PARSE_NOERROR);
    
    if (doc == NULL) {
        printf("Invalid Series ID\n");
        free(url);
        free(backdrops);
        free(season_banners);
        free(wide_season_banners);
        free(season_number_index);
        free(wide_season_number_index);
        return False;
    }
    
    /* Get the root element node */
    root_element = xmlDocGetRootElement(doc);
    
    /* 
     * Iterate through XML file, all relevent nodes are children of the root 
     * element (data) 
     */
    for (cur_node = root_element->children; cur_node; 
    cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && !strcmp((char *)
            cur_node->name, "Banner")) {
            path = (char *)malloc(MAX_URL_LEN*sizeof(char));
            path[0] = '\0';
            strcat(path, "http://www.thetvdb.com/banners/");
            ban_elems = xmlChildElementCount(cur_node);
            is_fan_art = False;
            is_season_ban = False;
            is_wide_season_ban = False;

            for (i = 0; i < ban_elems; i++) {
                if (i == 0) {
                    cur_ban_node = cur_node->children->next;
                } else {
                    cur_ban_node = cur_ban_node->next;
                }    	
                while(cur_ban_node->type != XML_ELEMENT_NODE) {
                    cur_ban_node = cur_ban_node->next;
                }
            
                if (strcmp((char *)cur_ban_node->name, "BannerPath") == 0) {
                    char *path_end = (char *)xmlNodeGetContent(cur_ban_node);
                    
                    strcat(path, path_end);
                    xmlFree(path_end);
                } else if (strcmp((char *)cur_ban_node->name, "BannerType") 
                    == 0) {
                    type = (char *)xmlNodeGetContent(cur_ban_node);
                    if (strcmp(type, "fanart") == 0) {
                        is_fan_art = True;
                    }
                    xmlFree(type);
                } else if (!(strcmp((char*)cur_ban_node->name,"BannerType2"))) {
                    type = (char *)xmlNodeGetContent(cur_ban_node);
                    if (strcmp(type, "season") == 0) {
                        is_season_ban = True;
                    } else if (!(strcmp(type, "seasonwide"))) {
                        is_wide_season_ban = True;
                    }
                    xmlFree(type);
                } else if (!(strcmp((char*)cur_ban_node->name,"Season"))) {
                    season_num = (char *)xmlNodeGetContent(cur_ban_node);
                    season_banner_num = atoi(season_num);
                }
            }
            if (is_fan_art) {
                backdrops[ban_index] = path;
                ban_index++;
            } else if (is_season_ban) {
                if (!(season_banners[season_banner_num])) {
                    season_banners[season_banner_num] = 
                        (char**)malloc(MAX_NUM_BAN*sizeof(char*));
                    for (j = 0; j < MAX_NUM_BAN; j++) {
                        season_banners[season_banner_num][j] = NULL;
                    }
                    num_season_banners++;
                }
                j = -1;
                while (season_banners[season_banner_num][++j]);
                season_banners[season_banner_num][j] = path;
                if (!season_number_index[season_banner_num]) {
                    season_number_index[season_banner_num] = season_num;
                } else {
                    xmlFree(season_num);
                }
                
            } else if (is_wide_season_ban) {
                if (!(wide_season_banners[season_banner_num])) {
                    wide_season_banners[season_banner_num] = 
                        (char**)malloc(MAX_NUM_BAN*sizeof(char*));
                    for (j = 0; j < MAX_NUM_BAN; j++) {
                        wide_season_banners[season_banner_num][j] = NULL;
                    }
                    num_wide_season_banners++;
                }
                j = -1;
                while (wide_season_banners[season_banner_num][++j]);
                wide_season_banners[season_banner_num][j] = path;
                if (!wide_season_number_index[season_banner_num]) {
                    wide_season_number_index[season_banner_num] = season_num;
                } else {
                    xmlFree(season_num);
                }
                
            }
            else {
                free(path);
            }
            
        }
    }
    xmlFreeDoc(doc);
    free(url);
    
    series_info->backdrops = backdrops;
    series_info->season_banners = season_banners;
    series_info->num_season_banners = num_season_banners;
    series_info->wide_season_banners = wide_season_banners;
    series_info->num_wide_season_banners = num_wide_season_banners;
    series_info->season_number_index = season_number_index;
    series_info->wide_season_number_index = wide_season_number_index;
    
    return True;
}

/*
* retrieve_series_info
* @series_ID:   ID of series information to be retrieved 
*
* Retrieves and parses the specified series information
*/
series_information* retrieve_series_info(char *series_ID)
{
    series_information *series_info = NULL;
    xmlNode *root_element = NULL; xmlNode *cur_ser_node = NULL;
    xmlNode *cur_node = NULL;
    xmlDoc *doc = NULL;
    int ser_elems, i, j, genre_index;
    char *url = (char*)malloc(MAX_URL_LEN*sizeof(char)); char *pch;

    sprintf(url, "http://thetvdb.com/data/series/%s", series_ID);
    
    /* parse the file and get the first DOM */
    doc = xmlReadFile(url, NULL, XML_PARSE_NOERROR);
    
    if (doc == NULL) {
        printf("Invalid Series ID\n");
        free(url);
        return NULL;
    }
    
    /* Get the root element node */
    root_element = xmlDocGetRootElement(doc);
    
    /* 
     * Iterate through XML file, all relevent nodes are children of the root 
     * element (data) 
     */
    for (cur_node = root_element->children; cur_node; 
    cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && !strcmp((char*)cur_node->name, 
        "Series")) {
            series_info = 
                (series_information *)malloc(sizeof(series_information));
            retrieve_series_backdrops(series_info);
            
            in_place_adjust_series_info(series_info);
            ser_elems = xmlChildElementCount(cur_node);
            
            for (i = 0; i < ser_elems; i++) {
                if (i == 0) {
                    cur_ser_node = cur_node->children->next;
                } else {
                    cur_ser_node = cur_ser_node->next;
                }    	
                
                while(cur_ser_node->type != XML_ELEMENT_NODE) {
                    cur_ser_node = cur_ser_node->next;
                }
            
                /* Determine values for series */
                if (strcmp((char*)cur_ser_node->name, "SeriesName") == 0) {
                    series_info->series_name = 
                    (char *)xmlNodeGetContent(cur_ser_node);
                } else if (strcmp((char*)cur_ser_node->name, "Genre") == 0) {
                    series_info->genre = (char **) malloc(MAX_NUM_GENRES * 
                    sizeof(char *));

                    for (j = 0; j < MAX_NUM_GENRES; j++) {
                        series_info->genre[j] = NULL;
                    }
                    pch = (char *)xmlNodeGetContent(cur_ser_node);
                    
                    if(!atof((char*)pch)) {
                        xmlFree(pch);
                        continue;
                    }
                    
                    series_info->genre[MAX_NUM_GENRES-1] = pch;
                    genre_index = 0;
                    if((pch = strtok(pch,"|")))
                    {
                        series_info->genre[genre_index] = pch;
                        genre_index++;
                    }
                    while (pch != NULL)
                    {
                        pch = strtok (NULL, "|");
                        series_info->genre[genre_index] = pch;
                        genre_index++;
                    }
                    
                } else if (strcmp((char*)cur_ser_node->name, 
                    "ContentRating") == 0) {
                    series_info->mpaa = (char *)xmlNodeGetContent(cur_ser_node);
                } else if (strcmp((char*)cur_ser_node->name, "Runtime") == 0) {
                    series_info->runtime = 
                    (char *)xmlNodeGetContent(cur_ser_node);
                }
            }
            /* Only one series element therefore Ignore other elements */       
            xmlFreeDoc(doc);
            free(url);
            return series_info;
        }
    }
    xmlFreeDoc(doc);
    free(url);
    return NULL;
}

/*
* parse_series_info_aux
* @series_doc:  XML Document of series
*
* Parses series information using the specified XML Docment.
*/
series_information* parse_series_info_aux(xmlDoc *series_doc)
{
    series_information *series_info = NULL;
    xmlNode *root_element = NULL; xmlNode *cur_ser_node = NULL;
    xmlNode *cur_node = NULL;
    int ser_elems, i, j, genre_index;
    char *pch;
    
    /* Get the root element node */
    root_element = xmlDocGetRootElement(series_doc);
    
    /* 
     * Iterate through XML file, all relevent nodes are children of the root 
     * element (data) 
     */
    for (cur_node = root_element->children; cur_node; 
    cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && !strcmp((char*)cur_node->name, 
        "Series")) {
            series_info = (series_information *)
            malloc(sizeof(series_information));
            ser_elems = xmlChildElementCount(cur_node);

            for (i = 0; i < ser_elems; i++) {
                if (i == 0) {
                    cur_ser_node = cur_node->children->next;
                } else {
                    cur_ser_node = cur_ser_node->next;
                }    	
                
                while(cur_ser_node->type != XML_ELEMENT_NODE) {
                    cur_ser_node = cur_ser_node->next;
                }
            
                /* Determine values for series */
                if (strcmp((char*)cur_ser_node->name, "id")==0) {
                    series_info->series_ID = 
                        (char*)xmlNodeGetContent(cur_ser_node);
                    retrieve_series_backdrops(series_info);
                    in_place_adjust_series_info(series_info);
                } else if (strcmp((char*)cur_ser_node->name, "SeriesName") == 0) {
                    series_info->series_name = 
                    (char *)xmlNodeGetContent(cur_ser_node);
                } else if (strcmp((char*)cur_ser_node->name, "Genre") == 0) {
                    series_info->genre = (char **) malloc(MAX_NUM_GENRES * 
                    sizeof(char *));

                    for (j = 0; j < MAX_NUM_GENRES; j++) {
                        series_info->genre[j] = NULL;
                    }
                    pch = (char *)xmlNodeGetContent(cur_ser_node);
                    series_info->genre[MAX_NUM_GENRES-1] = pch;
                    genre_index = 0;
                    if((pch = strtok(pch,"|")))
                    {
                        series_info->genre[genre_index] = pch;
                        genre_index++;
                    }
                    while ((pch = strtok (NULL, "|")))
                    {
                        series_info->genre[genre_index] = pch;
                        genre_index++;
                    }
                    
                } else if (strcmp((char*)cur_ser_node->name, 
                    "ContentRating") == 0) {
                    series_info->mpaa = (char *)xmlNodeGetContent(cur_ser_node);
                } else if (strcmp((char*)cur_ser_node->name, "Runtime") == 0) {
                    series_info->runtime = 
                    (char *)xmlNodeGetContent(cur_ser_node);
                }
            }
            /* Only one series element therefore Ignore other elements */       
            return series_info;
        }
    }

    return NULL;
}

/*
* retrieve_episode:
* @series_ID:   ID for series of episode being retrieved   
* @s_num:       Season number of episode
* @e_num:       Index of episode in season
* @dvd_order:   Use dvd sorted order
*
* Retrieves and parses the specified episode
*/
episode* retrieve_episode(char* series_ID, int s_num, int e_num, MYBOOL
dvd_order) 
{    
    xmlNode *root_element = NULL; xmlNode *cur_node = NULL;
    xmlDoc *doc = NULL;
    double len;
    char *url = (char*)malloc(MAX_URL_LEN*sizeof(char));
    char *s_num_string, *e_num_string;
    char *order_type = dvd_order ? "dvd" : "default";

    /* Convert season and episode number arguments to strings */
    double len_f;
    if (s_num != 0) {
        len_f = log10((double)s_num);
        len = fmod(len_f, 1) == 0 ? floor(len_f+1) : ceil(len_f);
        s_num_string = (char*)malloc((int)len+1*sizeof(char));
        sprintf(s_num_string, "%d", s_num);
    } else {
        s_num_string = (char*)malloc((int)2*sizeof(char));
        sprintf(s_num_string, "%d", s_num);
    }
    len_f = log10((double)e_num);
    len = fmod(len_f, 1) == 0 ? floor(len_f+1) : ceil(len_f);
    e_num_string = (char*)malloc((int)len+1*sizeof(char));
    sprintf(e_num_string, "%d", e_num);
    
    sprintf(url, 
        "http://thetvdb.com/api/3D32B0AB86EA0BB3/series/%s/%s/%s/%s", series_ID, 
        order_type, s_num_string, e_num_string);
    
    /*parse the file and get the first DOM */
    doc = xmlReadFile(url, NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    
    /* Try again with normal ordering only on first episode */
    if (doc == NULL && dvd_order && !invalid_order_type) {
        free(url);
        free(s_num_string);
        free(e_num_string);
        xmlFreeDoc(doc);
        printf("Unable to Retrieve Episode: Invalid Series ID, Season Number, "
            "Episode Number, or Order Type\nAttempting normal type...\n");
        invalid_order_type = True;
        retrieve_episode(series_ID, s_num, e_num, False);
    } else if (doc == NULL) {
        xmlFreeDoc(doc);
        free(url);
        free(s_num_string);
        free(e_num_string);
        return NULL;
    }

    /* Parse XML into Struct */
    
    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);
    
    /* 
     * Iterate through XML file, all relevent nodes are children of the root 
     * element (data) 
     */
    for (cur_node = root_element->children; cur_node; 
    cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && 
            !strcmp((char*)cur_node->name, "Episode")) {
            episode *new_episode = (episode *) malloc(sizeof(episode));
            
            xmlNode *cur_ep_node = NULL;
            int ep_elems = xmlChildElementCount(cur_node);
            
            int i;
            for (i = 0; i < ep_elems; i++) {
                if (i == 0) {
                    cur_ep_node = cur_node->children->next;
                } else {
                    cur_ep_node = cur_ep_node->next;
                }    	
                
                while(cur_ep_node->type != XML_ELEMENT_NODE) {
                    cur_ep_node = cur_ep_node->next;
                }

                /* Determine values for current episode */
                if (strcmp((char*)cur_ep_node->name, "EpisodeNumber") == 0 
                && !dvd_order) {
                    new_episode->episode_number = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                } else if (strcmp((char*)cur_ep_node->name, 
                    "DVD_episodenumber") == 0 
                && dvd_order) {
                    char *ep_num_float = (char *)
                        xmlNodeGetContent(cur_ep_node);
                    char* dvd_ep_num_string;
                    int dvd_ep_num = (int) atof(ep_num_float);
                    double len_float = log10((double)dvd_ep_num);
                    len = fmod(len_float, 1) == 0 ? floor(len_float+1) 
                    : ceil(len_float);

                    dvd_ep_num_string = (char*)
                        malloc((int)len+1 * sizeof(char));
                    sprintf(dvd_ep_num_string, "%d", dvd_ep_num);
                    new_episode->episode_number = dvd_ep_num_string;
                    
                    free(ep_num_float);
                } else if (strcmp((char*)cur_ep_node->name, "EpisodeName") 
                    == 0) {
                    new_episode->episode_name = 
                    (char *)xmlNodeGetContent(cur_ep_node);	
                } else if (strcmp((char*)cur_ep_node->name, "EpisodeName") 
                    == 0) {
                    new_episode->episode_name = 
                    (char *)xmlNodeGetContent(cur_ep_node);	
                } else if (strcmp((char*)cur_ep_node->name, "SeasonNumber") 
                    == 0 && 
                !dvd_order) {
                    new_episode->season_number = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                } else if (strcmp((char*)cur_ep_node->name, "DVD_season") 
                    == 0 && dvd_order) {
                    new_episode->season_number = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                } else if (strcmp((char*)cur_ep_node->name, 
                    "FirstAired") == 0) {
                    new_episode->firstaired = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                } else if (strcmp((char*)cur_ep_node->name, "Overview") == 0) {
                    new_episode->overview = 
                    (char *)xmlNodeGetContent(cur_ep_node);	
                } else if (strcmp((char*)cur_ep_node->name, "Director") == 0) {
                    int j, director_index;
                    char* pch;
                    new_episode->directors = (char **) 
                    malloc(MAX_NUM_DIRECTORS * 
                    sizeof(char *));

                    for (j = 0; j < MAX_NUM_DIRECTORS; j++) {
                        new_episode->directors[j] = NULL;
                    }
                    pch = (char *)xmlNodeGetContent(cur_ep_node);
                    new_episode->directors[MAX_NUM_DIRECTORS-1] = pch;
                    director_index = 0;
                    if((pch = strtok (pch,"|"))) {
                        new_episode->directors[director_index] = pch;
                        director_index++;
                    }
                    while ((pch != NULL)) {
                        pch = strtok (NULL, "|");
                        new_episode->directors[director_index] = pch;
                        director_index++;
                    }	
                } else if (strcmp((char*)cur_ep_node->name, "GuestStars") 
                    == 0) {
                    int j, actor_index;
                    char* pch;
                    new_episode->actors = (char **) malloc(MAX_NUM_ACTORS * 
                    sizeof(char *));

                    for (j = 0; j < MAX_NUM_ACTORS; j++) {
                        new_episode->actors[j] = NULL;
                    }
                    pch = (char *)xmlNodeGetContent(cur_ep_node);
                    new_episode->actors[MAX_NUM_ACTORS-1] = pch;
                    actor_index = 0;
                    if((pch = strtok (pch,"|"))) {
                        new_episode->actors[actor_index] = pch;
                        actor_index++;
                    }
                    while ((pch != NULL)) {
                        pch = strtok (NULL, "|");
                        new_episode->actors[actor_index] = pch;
                        actor_index++;
                    }  	
                } else if (strcmp((char*)cur_ep_node->name, "filename") == 0) {
                    char *backdrop_URL = (char *) malloc(MAX_URL_LEN * 
                    sizeof(char));
                    char *fileName = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                    sprintf(backdrop_URL, "http://www.thetvdb.com/banners/%s", 
                        fileName);
                    new_episode->backdrop = backdrop_URL;
                    xmlFree(fileName);
                }		
            }
            xmlFreeDoc(doc);
            free(url);
            free(s_num_string);
            free(e_num_string);
            return new_episode;	
        }
    }
    xmlFreeDoc(doc);
    free(url);
    free(s_num_string);
    free(e_num_string);
    return NULL;	
}

/*
* add_episode_to_series:
* @ep:          episode to add to series
* @ser:         series to be modified
* @low_bound:   lower bound of season, useful for years
*
* Adds episode to specified series.
*/
int add_episode_to_series(episode *ep, series *ser) {
    int season_num = atoi(ep->season_number);
    int episode_num = atoi(ep->episode_number);
    int i;
    /* Verify valid episodes */
    if (season_num > MAX_NUM_SEASONS || season_num < 0) {
        printf("Invalid season number: Greater than %d or less than 0\n", 
            MAX_NUM_SEASONS);
        printf("Skipping episode\n");
        return -1;
    }
    if (episode_num > MAX_NUM_EPS || episode_num < 0) {
        printf("Invalid episode number: Greater than %d or less than 0\n", 
            MAX_NUM_EPS);
        printf("Skipping episode\n");
        return -1;
    }
    /* Determine if episode's season has been allocated */
    if (!(ser->seasons[season_num])) {
        ser->seasons[season_num] = (season*)malloc(sizeof(season));
        ser->seasons[season_num]->episodes = 
            (episode**)malloc(MAX_NUM_EPS*sizeof(episode*));
        for (i = 0; i < MAX_NUM_EPS; i++) {
            ser->seasons[season_num]->episodes[i] = NULL;
        }
        ser->num_seasons++;
        ser->seasons[season_num]->num_episodes = 0;
    }
    ser->seasons[season_num]->episodes[episode_num] = ep;
    ser->seasons[season_num]->num_episodes++;
    return 0;
}

/*
* parse:
* @series_ID:   ID for series used on thetvdb
* @lang:        language of xml to be retrieved
* @dvd_order:   sort seasons of parsed episodes in dvd order   
*
* Parses specified series into a series struct.
*/
series* parse(char* series_ID, char* lang, MYBOOL dvd_order) {
    xmlNode *cur_node = NULL; xmlNode *root_element = NULL;
    xmlNode *cur_ep_node = NULL;
    episode *new_episode;
    double len;
    MYBOOL invalid_episode = False;
    series *new_series = (series*)malloc(sizeof(series));
    char *url = (char*)calloc(MAX_URL_LEN,sizeof(char));
    int i;
    xmlDoc *doc = NULL;
    new_series->seasons = (season**)malloc(MAX_NUM_SEASONS*sizeof(season*));
    
    /*
    * this initialize the library and check potential ABI mismatches
    * between the version it was compiled for and the actual shared
    * library used.
    */
    LIBXML_TEST_VERSION
    
    for (i = 0; i < MAX_NUM_SEASONS; i++) {
        new_series->seasons[i] = NULL;
    }
    
    sprintf(url, 
        "http://thetvdb.com/api/3D32B0AB86EA0BB3/series/%s/all/%s.xml", 
        series_ID, lang);
    new_series->num_seasons = 0;
    /*parse the file and get the first DOM */
    doc = xmlReadFile(url, NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

    if (!doc) {
        printf("Unable to retrieve series xml, verify series ID\n");
        free(new_series->seasons);
        free(new_series);
        free(url);
        
        /*
        * Free the global variables that may
        * have been allocated by the parser.
        */
        xmlCleanupParser();
  
        return NULL;
    }

    if (!(new_series->series_info = parse_series_info_aux(doc))) {
        printf("Series Info set to NULL: "
            "Unable to retrieve series information.\n");
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);
    
    /* 
    * Iterate through XML file, all relevent nodes are children of the 
    * root element (data) 
    */
    for (cur_node = root_element->children; cur_node; 
        cur_node = cur_node->next) {
        
        if (cur_node->type == XML_ELEMENT_NODE && 
            !strcmp((char*)cur_node->name, "Episode")) {
            int ep_elems = xmlChildElementCount(cur_node);
            
            cur_ep_node = NULL;
            new_episode = init_episode();
            
            for (i = 0; i < ep_elems; i++) {
                if (i == 0) {
                    cur_ep_node = cur_node->children->next;
                } else {
                    cur_ep_node = cur_ep_node->next;
                }    	
                
                while(cur_ep_node->type != XML_ELEMENT_NODE) {
                    cur_ep_node = cur_ep_node->next;
                    
                }
                /* Determine values for current episode */
                if (strcmp((char*)cur_ep_node->name, "EpisodeNumber") == 0 
                && !dvd_order) {
                    char *ep_num = (char*)xmlNodeGetContent(cur_ep_node);
                    if(!strcmp(ep_num,"")) {
                        printf("Invalid episode number: Null or Empty\n"
                            "Skipping episode\n");
                        xmlFree(ep_num);
                        free_episode(new_episode);

                        invalid_episode = True;
                        break;
                    }
                    new_episode->episode_number = ep_num;
                } else if (strcmp((char*)cur_ep_node->name, 
                    "DVD_episodenumber") == 0 
                    && dvd_order) {
                    int dvd_ep_num;
                    double len_float;
                    char *ep_num_float, *dvd_ep_num_string;
                    ep_num_float = (char*)xmlNodeGetContent(cur_ep_node);
                    if(!strcmp(ep_num_float,"")) {
                        printf("Invalid episode number: Null or Empty\n"
                            "Skipping episode\n");
                        xmlFree(ep_num_float);
                        free_episode(new_episode);

                        invalid_episode = True;
                        break;
                    }
                    
                    dvd_ep_num = (int) atof(ep_num_float);
                    len_float = log10((double)dvd_ep_num);
                    len = fmod(len_float, 1) == 0 ? floor(len_float+1) 
                    : ceil(len_float);
                    
                    dvd_ep_num_string = (char*)
                        malloc((int)len+1 * sizeof(char));
                    sprintf(dvd_ep_num_string, "%d", dvd_ep_num);
                    new_episode->episode_number = dvd_ep_num_string;
                    
                    free(ep_num_float);
                } else if (strcmp((char*)cur_ep_node->name, "EpisodeName") 
                    == 0) {
                    new_episode->episode_name = 
                    (char *)xmlNodeGetContent(cur_ep_node);	
                } else if (strcmp((char*)cur_ep_node->name, "EpisodeName") 
                    == 0) {
                    new_episode->episode_name = 
                    (char *)xmlNodeGetContent(cur_ep_node);	
                } else if (strcmp((char*)cur_ep_node->name, "SeasonNumber") 
                    == 0 && !dvd_order) {
                    char* season_num = (char*)xmlNodeGetContent(cur_ep_node);
                    if(!strcmp(season_num,"")) {
                        printf("Invalid season number: Null or Empty\n"
                            "Skipping episode\n");
                        xmlFree(season_num);
                        free_episode(new_episode);

                        invalid_episode = True;
                        break;
                    }
                    new_episode->season_number = season_num;
                } else if (strcmp((char*)cur_ep_node->name, "DVD_season") 
                    == 0 && dvd_order) {
                    char* season_num = (char*)xmlNodeGetContent(cur_ep_node);
                    if(!strcmp(season_num,"")) {
                        printf("Invalid season number: Null or Empty\n"
                            "Skipping episode\n");
                        xmlFree(season_num);
                        free_episode(new_episode);

                        invalid_episode = True;
                        break;
                    }
                    new_episode->season_number = season_num;
                } else if (strcmp((char*)cur_ep_node->name, 
                    "FirstAired") == 0) {
                    new_episode->firstaired = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                } else if (strcmp((char*)cur_ep_node->name, "Overview") == 0) {
                    new_episode->overview = 
                    (char *)xmlNodeGetContent(cur_ep_node);	
                } else if (strcmp((char*)cur_ep_node->name, "Director") == 0) {
                    int j, director_index;
                    char* pch;
                    new_episode->directors = (char **) 
                    malloc(MAX_NUM_DIRECTORS*sizeof(char *));
                    for (j = 0; j < MAX_NUM_DIRECTORS; j++) {
                        new_episode->directors[j] = NULL;
                    }
                    pch = (char *)xmlNodeGetContent(cur_ep_node);
                    if(!atof((char*)pch)) {
                        xmlFree(pch);
                        continue;
                    }
                    
                    new_episode->directors[MAX_NUM_DIRECTORS-1] = pch;
                    director_index = 0;
                    if((pch = strtok (pch,"|"))) {
                        new_episode->directors[director_index] = pch;
                        director_index++;
                    } else {
                        xmlFree(pch);
                        continue;
                    }
                    while ((pch = strtok (NULL, "|"))) {
                        new_episode->directors[director_index] = pch;
                        director_index++;
                    }	
                } else if (strcmp((char*)cur_ep_node->name, "GuestStars") 
                    == 0) {
                    int j, actor_index;
                    char* pch;
                    new_episode->actors = (char **)malloc(MAX_NUM_ACTORS* 
                        sizeof(char *));
                    for (j = 0; j < MAX_NUM_ACTORS; j++) {
                        new_episode->actors[j] = NULL;
                    }
                    pch = (char *)xmlNodeGetContent(cur_ep_node);
                    if(!atof((char*)pch)) {
                        xmlFree(pch);
                        continue;
                    }
                    new_episode->actors[MAX_NUM_ACTORS-1] = pch;
                    actor_index = 0;
                    if((pch = strtok (pch,"|"))) {
                        new_episode->actors[actor_index] = pch;
                        actor_index++;
                    } else {
                        xmlFree(pch);
                        continue;
                    }
                    while ((pch = strtok (pch,"|"))) {
                        new_episode->actors[actor_index] = pch;
                        actor_index++;
                    }  	
                } else if (strcmp((char*)cur_ep_node->name, "filename") == 0) {
                    char *backdrop_URL = (char *) malloc(MAX_URL_LEN * 
                    sizeof(char));
                    char *file_name = 
                    (char *)xmlNodeGetContent(cur_ep_node);
                    sprintf(backdrop_URL, "http://www.thetvdb.com/banners/%s", 
                        file_name);
                    if(!strcmp(file_name,"")) {
                        sprintf(backdrop_URL, "N/A");
                    }
                    new_episode->backdrop = backdrop_URL;
                    xmlFree(file_name);
                }	 
            }
            if (!invalid_episode){
                if (add_episode_to_series(new_episode, new_series) < 0) {
                    free_episode(new_episode);
                }
            } else {
                invalid_episode = False;
            }
        }
    }
    
    if (new_series->num_seasons == 0 && dvd_order) {
        printf("Invalid Series: No seasons found.\n"
            "Retrying with normal order...\n");
        xmlFreeDoc(doc);
        free(url);
        free(new_series->seasons);
        free_series_information(new_series->series_info);
        free(new_series);
        
        xmlCleanupParser();
        
        return parse(series_ID, lang, False);
    } else if (new_series->num_seasons == 0) {
        printf("Invalid Series: No seasons found.\n");
        xmlFreeDoc(doc);
        free(url);
        free(new_series->seasons);
        free_series_information(new_series->series_info);
        free(new_series);
        
        xmlCleanupParser();
        
        return NULL;
    } else {
        int j;
        xmlFreeDoc(doc);
        free(url);
        
        /* Readjust series */
        in_place_adjust_series(new_series);
        
        /* Readjust seasons */
        for (j = 0; j < new_series->num_seasons; j++) {
            in_place_adjust_season(new_series->seasons[j]);
        }
        
        xmlCleanupParser();

        return new_series;
    }
}
/*
int main(int argc, char **argv)
{
    series *test_series;

    
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     
    LIBXML_TEST_VERSION

    if (!(test_series = parse("83214", "en", True))) {
        xmlCleanupParser();
        exit(1);
    } 
    
    free_series(test_series);
    
    
     * Free the global variables that may
     * have been allocated by the parser.
     
    xmlCleanupParser();
    
    printf("Parse Successful\n");

    return 0;
}
#else
int main(void) 
{
    fprintf(stderr, "Tree support not compiled in\n");
    exit(1);
}*/
#endif
