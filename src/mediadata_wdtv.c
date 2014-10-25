/*
 * mediadata_wdtv.c
 * Makes use of the MediaData thetvdb parser in order to generate XML documents 
 * formatted for the WDTV media player.
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "getopt.h"
#include "writer.h"

static int MAX_DIR_LEN = 1024;
static int MAX_FN_LEN = 255;

#if WIN32
MAX_DIR_LEN = 255;
MAX_FN_LEN = 255;
#endif

char* parse_path(episode *episode, char* series_title, char* path,
                        char* format) 
{
    char* episode_num = episode->episode_number;
    char* season_num = episode->season_number;
    char* episode_fn = (char*)calloc(MAX_FN_LEN,sizeof(char));
    char* episode_path = (char*)calloc((MAX_FN_LEN+MAX_DIR_LEN),sizeof(char));
    char* cur_string = (char*)malloc(2*sizeof(char));
    char* slash = "/";
    char* valid_series_title = (char*)calloc(MAX_FN_LEN,sizeof(char));
    char cur;
    int i = 0;
    int j = 0;
    int k = 0;
    char c;
    MYBOOL en_less_than_nine = strlen(episode_num) == 1 ? True : False;
    MYBOOL sn_less_than_nine = strlen(season_num) == 1 ? True : False;
    
    char* zero_episode_num = (char*)malloc(strlen(episode_num)+2*sizeof(char));
    char* zero_season_num = (char*)malloc(strlen(season_num)+2*sizeof(char));
    
    /* Replace invalid filename characters
     * < > : " / \ | ? * */
    while(series_title[j]) {
        c = series_title[j];
        if (series_title[j-1] != ' ' && c == ':' && series_title[j+1] == ' ') {
            valid_series_title[k++] = ' ';
            valid_series_title[k++] = '-';
        } else if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/'
                || c == '\\' || c == '|' || c == '?' || c == '*')
            valid_series_title[k++] = '_';
        else
            valid_series_title[k++] = series_title[j];
        j++;
    }

    sprintf(zero_episode_num, "%d%s", 0, episode_num);
    sprintf(zero_season_num, "%d%s", 0, season_num);
    /* Parse user-specified filename format */
    while ((cur = format[i++])!='\0') {
        if (cur == '+') {
            if (format[i]=='*') {
                if (format[i+1]=='S') {
                    /* Place Season Number here with zero before */
                    strcat(episode_fn, "S");
                    if (sn_less_than_nine)
                        strcat(episode_fn, zero_season_num);
                    else
                        strcat(episode_fn, season_num);
                    i+=2;
                } else if (format[i+1]=='s') {
                    /* Place Season Number here */
                    strcat(episode_fn, "S");
                    strcat(episode_fn, season_num);
                    i+=2;
                } else if (format[i+1]=='E') {
                    /* Place episode number with zero before here */
                    strcat(episode_fn, "E");
                    if (en_less_than_nine)
                        strcat(episode_fn, zero_episode_num);
                    else
                        strcat(episode_fn, episode_num);
                    i+=2;
                } else if (format[i+1]=='e') {
                    /* Place episode number here */
                    strcat(episode_fn, "E");
                    strcat(episode_fn, episode_num);
                    i+=2;
                }
            } else if (format[i]=='S') {
                /* Place Season Number here with zero before */
                strcat(episode_fn, "s");
                if (sn_less_than_nine)
                    strcat(episode_fn, zero_season_num);
                else
                    strcat(episode_fn, season_num);
                i++;
            } else if (format[i]=='s') {
                /* Place Season Number here */
                strcat(episode_fn, "s");
                strcat(episode_fn, season_num);
                i++;
            } else if (format[i]=='E') {
                /* Place episode number with zero before here */
                strcat(episode_fn, "e");
                if (en_less_than_nine)
                    strcat(episode_fn, zero_episode_num);
                else
                    strcat(episode_fn, episode_num);
                i++;
            } else if (format[i]=='e') {
                /* Place episode number here */
                strcat(episode_fn, "e");
                strcat(episode_fn, episode_num);
                i++;
            } else if (format[i]=='T') {
                /* place show title here */
                strcat(episode_fn, valid_series_title);
                i++;
            } else {
                /* Place + here */
                strcat(episode_fn, "+");
            }
        } else {
            /* Place cur here */
            sprintf(cur_string, "%c", cur);
            strcat(episode_fn, cur_string);
        }
    }

    #if WIN32
    if (strlen(episode_fn) + strlen(path) + 2 > MAX_DIR_LEN) {
        printf("Windows supports a path plus file name length of at most ");
        printf("%d characters.\n", MAX_DIR_LEN);
        free(zero_episode_num);
        free(zero_season_num);
        free(episode_fn);
        free(episode_path);
        free(cur_string);
        free(valid_series_title);
        return NULL;
    }
    #endif
    
    if (strlen(episode_fn) + strlen(path) + 2 > MAX_DIR_LEN+MAX_FN_LEN) {
        printf("File path of %d char plus file name too long.\n", MAX_DIR_LEN);
        free(zero_episode_num);
        free(zero_season_num);
        free(episode_fn);
        free(episode_path);
        free(cur_string);
        free(valid_series_title);
        return NULL;
    }
    

/*
    #if defined(__MINGW32__) || defined(WIN32)
    slash = "\\";
    #endif*/

    sprintf(episode_path, "%s%s%s", path, slash, episode_fn);
    free(zero_episode_num);
    free(zero_season_num);
    free(episode_fn);
    free(cur_string);
    free(valid_series_title);

    return episode_path;
}

int copy_file(char *old_filename, char  *new_filename)
{
    FILE  *ptr_old, *ptr_new;
    int err = 0; int err1 = 0;
    int  a;

    char *subbuff_old = (char*)calloc(8,sizeof(char));
    char *real_loc_old;
    char *subbuff_new = (char*)calloc(8,sizeof(char));
    char *real_loc_new;
    memcpy( subbuff_old, &old_filename[0], 7 );
    subbuff_old[7] = '\0';

    if (!(strcmp(subbuff_old, "file://"))) {
        real_loc_old = &old_filename[7];
        #if defined(__MINGW32__) || defined(WIN32)
        real_loc_old = &old_filename[8];
        #endif
    } else {
        real_loc_old = old_filename;
    }
    
    old_filename = real_loc_old;
    
    memcpy( subbuff_new, &new_filename[0], 7 );
    subbuff_new[7] = '\0';
    
    if (!(strcmp(subbuff_new, "file://"))) {
        real_loc_new = &new_filename[7];
        #if defined(__MINGW32__) || defined(WIN32)
        real_loc_new = &new_filename[8];
        #endif
    } else {
        real_loc_new = new_filename;
    }

    new_filename = real_loc_new;
    ptr_old = fopen(old_filename, "rb");
    ptr_new = fopen(new_filename, "wb");

    if(err != 0) {
        return  -1;
    }

    if(err1 != 0)
    {
        fclose(ptr_old);
        return  -1;
    }

    while(1)
    {

        a  =  fgetc(ptr_old);

        if(!feof(ptr_old))
            fputc(a, ptr_new);
        else
            break;
    }

    fclose(ptr_new);
    fclose(ptr_old);

    free(subbuff_old);
    free(subbuff_new);

    return  0;
}

MYBOOL write_ep(series *parsed_series, char *season_num,
                          char *episode_num, char*path, char *format) 
{
    int num_seasons = parsed_series->num_seasons;
    int num_episodes;
    int num_season_banners = parsed_series->series_info->num_season_banners;
    int i, j, k;
    series_information *series_info = parsed_series->series_info;
    char *title = series_info->series_name;
    char *file_path, *file_path2;
    episode *ep;
    
    /* Determine index for specified season number */
    i= -1;
    while (parsed_series->seasons[++i]) {
        if (!(strcmp(season_num, 
            parsed_series->seasons[i]->episodes[0]->season_number)))
            break;
    }
        
    if (i == num_seasons) {
        printf("Invalid Season Specified: Unable to find season %s\n", 
            season_num);
        return False;
    }
    
    num_episodes = parsed_series->seasons[i]->num_episodes;
    
    /* Determine index of banners for specified season number */
    k= -1;
    while (parsed_series->series_info->season_number_index[++k] &&
        (strcmp(season_num, parsed_series->series_info->
        season_number_index[k])));
    if (k == num_season_banners) {
        printf("Warning: Unable to find season banner for season %s\n", 
            season_num);
        k = -1;
    }
    
    if (episode_num) {
        /* Determine index for specified episode number */
        j = -1;
        while (parsed_series->seasons[i]->episodes[++j]) {
            if (!(strcmp(episode_num, 
                parsed_series->seasons[i]->episodes[j]->episode_number)))
                break;
        }
    } else { j = 0; };
    
    if (j == num_episodes) {
        printf("Invalid Episode Specified: Unable to find Episode %s\n", 
            episode_num);
        return False;
    }

    ep = parsed_series->seasons[i]->episodes[j];
    file_path = parse_path(ep, title, path, format);
    file_path2 = parse_path(ep, title, path, format);
    strcat(file_path, ".xml");
    strcat(file_path2, ".jpg");
    write_episode(ep, series_info, file_path);
    


    if (k != -1)
        download_url(parsed_series->series_info->season_banners[k][0], 
            file_path2);
    
    free(file_path);
    free(file_path2);
    
    return True;                    
}

MYBOOL write_season(series* parsed_series, char *season_num,
                         char *episode_num, char *path, char *format, MYBOOL c)
{
    int num_seasons = parsed_series->num_seasons;
    int num_season_banners = parsed_series->series_info->num_season_banners;
    int i, j, orig_j, k, num_episodes;
    char *file_path;
    char *file_path2;
    episode *cur_ep;
    series_information *series_info = parsed_series->series_info;
    char *title = series_info->series_name;
    char *season_path;
    
    /* Determine index for specified season number */
    i= -1;
    while (parsed_series->seasons[++i]) {
        if (!(strcmp(season_num, 
            parsed_series->seasons[i]->episodes[0]->season_number)))
            break;
    }
        
    if (i == num_seasons) {
        printf("Invalid Season Specified: Unable to find season %s\n", 
            season_num);
        return False;
    }
    
    num_episodes = parsed_series->seasons[i]->num_episodes;

    /* Determine index of banners for specified season number */
    k= -1;
    while (parsed_series->series_info->season_number_index[++k] &&
        (strcmp(season_num, parsed_series->series_info->
        season_number_index[k])));
    if (k == num_season_banners) {
        printf("Warning: Unable to find season banner for season %s\n",
            season_num);
        k = -1;
    }


    if (episode_num) {
        /* Determine index for specified episode number */
        j = -1;
        while (parsed_series->seasons[i]->episodes[++j]) {
            if (!(strcmp(episode_num, 
                parsed_series->seasons[i]->episodes[j]->episode_number)))
                break;
        }
    } else { j = 0;};
    
    orig_j = j;
    
    if (j == num_episodes) {
        printf("Invalid Episode Specified: Unable to find Episode %s\n", 
            episode_num);
        return False;
    }
    
    
    for (; j < num_episodes; j++) {
        cur_ep = parsed_series->seasons[i]->episodes[j];
        file_path = parse_path(cur_ep, title, path, format);
        file_path2 = parse_path(cur_ep, title, path, format);
        strcat(file_path, ".xml");
        strcat(file_path2, ".jpg");
        write_episode(cur_ep, series_info, file_path);
        if (j == orig_j) {
            season_path = parse_path(cur_ep, title, path, format);
            strcat(season_path, ".jpg");
            if (k != -1)
                download_url(parsed_series->series_info->season_banners[k][0], 
                    season_path);
        } else {
            if (k != -1)
                copy_file(season_path, file_path2);
        }
        free(file_path);
        free(file_path2);
    }
    free(season_path);
    
    /* Complete the series */
    if (c) {
        for (++i; i < num_seasons; i++) {
            num_episodes = parsed_series->seasons[i]->num_episodes;
            /* Determine index of banners for specified season number */
            season_num = parsed_series->seasons[i]->episodes[0]->season_number;
            k= -1;
            while (parsed_series->series_info->season_number_index[++k] &&
                (strcmp(season_num, parsed_series->series_info->
                season_number_index[k])));
            if (k == num_season_banners) {
                printf("Warning: Unable to find season banner for season %s\n", 
                    season_num);
                k = -1;
            }
            for (j = 0; j < num_episodes; j++) {
                cur_ep = parsed_series->seasons[i]->episodes[j];
                file_path = parse_path(cur_ep, title, path, format);
                file_path2 = parse_path(cur_ep, title, path, format);
                strcat(file_path, ".xml");
                strcat(file_path2, ".jpg");
                write_episode(cur_ep, series_info, file_path);
                if (j == 0) {
                    season_path = parse_path(cur_ep, title, path, format);
                    strcat(season_path, ".jpg");
                    if (k != -1)
                        download_url(parsed_series->series_info->
                            season_banners[k][0], season_path);
                } else {
                    if (k != -1)
                        copy_file(season_path, file_path2);
                }
                
                free(file_path);
                free(file_path2);
            }
            free(season_path);
        }
    }

    return True;
}

void write_series(series* parsed_series, char *path, char *format)
{
    int num_seasons = parsed_series->num_seasons;
    char *season_num;
    int num_season_banners = parsed_series->series_info->num_season_banners;
    int i, j, k, num_episodes;
    char *file_path;
    char *file_path2;
    episode *cur_ep;
    series_information *series_info = parsed_series->series_info;
    char *title = series_info->series_name;
    char *season_path;
    for (i = 0; i < num_seasons; i++) {
        num_episodes = parsed_series->seasons[i]->num_episodes;
        /* Determine index of banners for specified season number */
        season_num = parsed_series->seasons[i]->episodes[0]->season_number;
        k= -1;
        while (parsed_series->series_info->season_number_index[++k] &&
            (strcmp(season_num, parsed_series->series_info->
            season_number_index[k])));

        if (k == num_season_banners) {
            printf("Warning: Unable to find season banner for season %s\n", 
                season_num);
            k = -1;
        }
        for (j = 0; j < num_episodes; j++) {
            cur_ep = parsed_series->seasons[i]->episodes[j];
            file_path = parse_path(cur_ep, title, path, format);
            file_path2 = parse_path(cur_ep, title, path, format);
            strcat(file_path, ".xml");
            strcat(file_path2, ".jpg");
            write_episode(cur_ep, series_info, file_path);
            if (j == 0) {
                season_path = parse_path(cur_ep, title, path, format);
                strcat(season_path, ".jpg");
                if (k != -1)
                    download_url(parsed_series->series_info->
                        season_banners[k][0], season_path);
            } else {
                if (k != -1)
                    copy_file(season_path, file_path2);
            }
            
            free(file_path);
            free(file_path2);
            
        }
        free(season_path);
    }

}

#ifdef CLI
int main(int argc, char **argv)
{
    int i, j, cnt;
    char cur, runner;
    int opt;
    char  *s_num, *e_num;
    MYBOOL complete = False;
    char *pstr;
    char *lang;
    char *output_dir;
    MYBOOL dvd_order = False;
    series *parsed_series;
    char *format = "+T.+*S+*E";
    
    pstr = NULL;
    lang = "en";
    output_dir = "";
    s_num = NULL;
    e_num = NULL;
    while ((opt = getopt(argc, argv, "i:l:cde:f:o:s:")) != -1) {
        switch (opt) {
            case 'i' : pstr = optarg;           break;
            case 'l' : lang = optarg;           break;
            case 'c' : complete = True;         break;
            case 'd' : dvd_order = True;        break;
            case 'e' : e_num = optarg;          break;
            case 'f' : format = optarg;         break;
            case 'o' : output_dir = optarg;  
                if (strlen(output_dir) > MAX_DIR_LEN) {
                    printf("Invalid directory: Path must be less than %d "
                    "characters. Try again...\n", MAX_DIR_LEN);
                    free(output_dir);
                    exit(EXIT_FAILURE);
                }                
                break;
            case 's' : s_num = optarg;    break;
            default: printf("Error: Unexpected case in switch()\n"); 
                exit(EXIT_FAILURE);
        }
    }

    if (!pstr) {
        printf("Expected valid argument(s). Try again...\n");
        exit(EXIT_FAILURE);
    }

    if (!(parsed_series = parse(pstr, lang, dvd_order))) {
        exit(EXIT_FAILURE);
    }

    i=0;
    cnt=0;
    /* Replace "%20" with spaces in path */
    while ((cur = output_dir[i++])!='\0') {
        if (cur == '%') {
            if (output_dir[i]=='2') {
                if (output_dir[i+1]=='0') {
                    output_dir[i-1] = ' ';
                    cnt++;
                    j = i;
                    while ((runner = output_dir[j])!='\0'
                            && j < strlen(output_dir)) {
                        output_dir[j] = output_dir[j+2];
                        output_dir[j+1] = output_dir[j+3];
                        j+=2;
                    }
                }
            }
        }
    }
    output_dir[i-cnt*2+1] = '\0';

    i = 0;
    cnt=0;
    /* Replace "%20" with spaces in format */
    while ((cur = format[i++])!='\0') {
        if (cur == '%') {
            if (format[i]=='2') {
                if (format[i+1]=='0') {
                    format[i-1] = ' ';
                    cnt++;
                    j = i;
                    while ((runner = format[j])!='\0' && j < strlen(format)) {
                        format[j] = format[j+2];
                        format[j+1] = format[j+3];
                        j+=2;
                    }
                }
            }
        }
    }
    format[i-cnt*2+1] = '\0';

    if (s_num && e_num && !complete) {
        write_ep(parsed_series, s_num, e_num, output_dir, format);
    } else if (s_num) {
        write_season(parsed_series, s_num, e_num, output_dir, format, complete);
    } else if (e_num) {
        printf("Season must be specified alongside episode. Try again...\n");
    } else {
        write_series(parsed_series, output_dir, format);
    }

    free_series(parsed_series);
    
    exit(EXIT_SUCCESS);
}
#endif
