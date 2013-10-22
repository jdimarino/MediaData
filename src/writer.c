/*
 * writer.c
 * Writer for thetvdb.com's web API modified for generating XML documents 
 * formatted for the WDTV.
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
 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "writer.h"

#define MY_ENCODING "UTF-8"

#if defined(LIBXML_WRITER_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

/**
 * null_check
 * @episode_to_check: the episode to check for null values
 * @series_info_to_check: the series info to check for null values
 *
 * Replaces null attributes with empty strings.
 */
void null_check(episode *episode_to_check,
        series_information *series_info_to_check)
{
    /* Check episode */
    episode_to_check->episode_name = episode_to_check->episode_name
            ? episode_to_check->episode_name : "";
    episode_to_check->episode_number = episode_to_check->episode_number
            ? episode_to_check->episode_number : "";
    episode_to_check->season_number = episode_to_check->season_number
            ? episode_to_check->season_number : "";
    episode_to_check->firstaired = episode_to_check->firstaired
            ? episode_to_check->firstaired : "";
    episode_to_check->overview = episode_to_check->overview
            ? episode_to_check->overview : "";
    episode_to_check->directors[0] = episode_to_check->directors[0]
            ? episode_to_check->directors[0] : "";
    episode_to_check->actors[0] = episode_to_check->actors[0]
            ? episode_to_check->actors[0] : "";

    /* Check series info */
    series_info_to_check->series_ID = series_info_to_check->series_ID
            ? series_info_to_check->series_ID : "";
    series_info_to_check->series_name = series_info_to_check->series_name
            ? series_info_to_check->series_name : "";
    series_info_to_check->genre[0] = series_info_to_check->genre[0]
            ? series_info_to_check->genre[0] : "";
    series_info_to_check->mpaa = series_info_to_check->mpaa
            ? series_info_to_check->mpaa : "";
    series_info_to_check->runtime = series_info_to_check->runtime
            ? series_info_to_check->runtime : "";
}

/**
* write_episode
* @episode_to_write: the episode to be written
* @return: BOOL set to True if write was successful, False otherwise
*
* Writes given episode stucture to xml documented formatted for the WDTV Live
**/
MYBOOL write_episode(episode *episode_to_write,
                      series_information *series_info_to_write, char* path)
{
    int rc, i;
    xmlTextWriterPtr writer;
    const xmlChar MY_INDENT_STR[5] = "	";
    char *cur_genre, *cur_director, *cur_actor, *cur_backdrop;
    char *ep_backdrop;
    LIBXML_TEST_VERSION

    null_check(episode_to_write, series_info_to_write);

    /* Create a new XmlWriter for path, with no compression. */
    writer = xmlNewTextWriterFilename(path, 0);
    if (writer == NULL) {
        printf("writer: Error creating the xml writer %s\n", path);
        return False;
    }
        
    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
    rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartDocument\n");
        return False;
    }

    /* Write document description as comment */
    rc = xmlTextWriterWriteComment(writer, BAD_CAST " XML Generated for the " 
        "WDTV by MediaData ");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterWriteComment\n");
        return False;
    }
    
    /* Write newline */
    rc = xmlTextWriterWriteFormatRaw(writer, "\n");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    /* Write the "details" root element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "details");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Newline workaround */
    xmlTextWriterSetIndent(writer, True); 
    xmlTextWriterSetIndentString(writer, BAD_CAST "");
    
    /* Write the "id" root element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "id");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        series_info_to_write->series_ID);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "id". */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Start indenting one tab here */
    xmlTextWriterSetIndentString(writer, MY_INDENT_STR);
    
    /* Write the "title" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "title");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s%s - %s",
            strtol(episode_to_write->episode_number, NULL, 10) < 10 ? "0" : "",
                    episode_to_write->episode_number,
                    episode_to_write->episode_name);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "title". */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "series_name" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "series_name");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        BAD_CAST series_info_to_write->series_name);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "series_name". */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "mpaa" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "mpaa");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        series_info_to_write->mpaa);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "mpaa". */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "episode_name" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "episode_name");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        episode_to_write->episode_name);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "episode_name". */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "season_number" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "season_number");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        episode_to_write->season_number);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "season_number" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "episode_number" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "episode_number");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        episode_to_write->episode_number);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "episode_number" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "firstaired" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "firstaired");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        episode_to_write->firstaired);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "firstaired" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "year" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "year");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        episode_to_write->firstaired);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "year" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    


    i = -1;
    
    while((cur_genre = series_info_to_write->genre[++i])) {
        /* Write the "genre" element */
        rc = xmlTextWriterStartElement(writer, BAD_CAST "genre");
        if (rc < 0) {
            printf("testXmlwriterFilename: "
                    "Error at xmlTextWriterStartElement\n");
            return False;
        }
        
        rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        cur_genre);
        if (rc < 0) {
            printf("testXmlwriterFilename:"
            "Error at xmlTextWriterStartElement\n");
            return False;
        }
        
        /* Close the element named "genre" */
        rc = xmlTextWriterEndElement(writer);
        if (rc < 0) {
            printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
            return False;
        }
    }
    

    
    /* Write the "runtime" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "runtime");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        series_info_to_write->runtime);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    
    /* Close the element named "runtime" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "plot" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "plot");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "\"%s\"- %s",
        episode_to_write->episode_name, episode_to_write->overview);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "plot" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    
    /* Write the "overview" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "overview");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "\"%s\"- %s",
        episode_to_write->episode_name, episode_to_write->overview);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "overview" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }

    i = -1;
    
    
    
    /* Write the "director" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "director");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }

    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
            episode_to_write->directors[++i]);
    if (rc < 0) {
        printf("testXmlwriterFilename:"
        "Error at xmlTextWriterStartElement\n");
        return False;
    }

    while((cur_director = episode_to_write->directors[++i])) {
        rc = xmlTextWriterWriteFormatRaw(writer, " / %s",
        cur_director);
        if (rc < 0) {
            printf("testXmlwriterFilename:"
            "Error at xmlTextWriterStartElement\n");
            return False;
        }
        
    }

    /* Close the element named "director" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }


    i = -1;


    
    /* Write the "actor" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "actor");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
            episode_to_write->actors[++i]);
    if (rc < 0) {
        printf("testXmlwriterFilename:"
        "Error at xmlTextWriterStartElement\n");
        return False;
    }

    while((cur_actor = episode_to_write->actors[++i])) {

        rc = xmlTextWriterWriteFormatRaw(writer, " / %s",
        cur_actor);
        if (rc < 0) {
            printf("testXmlwriterFilename:"
            "Error at xmlTextWriterStartElement\n");
            return False;
        }
        
    }
        
    /* Close the element named "actor" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }
    

    
    /* Write the episode "backdrop" element */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "backdrop");
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    ep_backdrop = episode_to_write->backdrop;
    if (!strcmp(ep_backdrop,"N/A")) {
        ep_backdrop = "";
    }
    rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        ep_backdrop);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return False;
    }
    
    /* Close the element named "backdrop" for episodes */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }

    i = -1;
    while((cur_backdrop = series_info_to_write->backdrops[++i])) {
    
        /* Write the series_information "backdrop" element */
        rc = xmlTextWriterStartElement(writer, BAD_CAST "backdrop");
        if (rc < 0) {
            printf("testXmlwriterFilename: "
                    "Error at xmlTextWriterStartElement\n");
            return False;
        }
    
        rc = xmlTextWriterWriteFormatRaw(writer, "%s",
        cur_backdrop);
        if (rc < 0) {
            printf("testXmlwriterFilename:"
            "Error at xmlTextWriterStartElement\n");
            return False;
        }
        
        /* Close the element named "backdrop" for series_information */
        rc = xmlTextWriterEndElement(writer);
        if (rc < 0) {
            printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
            return False;
        }
        
    }
    

    
    /* Close the element named "details" */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return False;
    }

    /* Here we could close the elements ORDER and EXAMPLE using the
     * function xmlTextWriterEndElement, but since we do not want to
     * write any other elements, we simply call xmlTextWriterEndDocument,
     * which will do all the work. 
     */
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
        return False;
    }

    xmlFreeTextWriter(writer);
    
    xmlCleanupParser();
    xmlMemoryDump();

    return True;
}

/* Unit Testing 
int main(void)
{

    episode *test_episode = (episode *)malloc(sizeof(episode));
    series_information *test_series_info = (series_information *)
    malloc(sizeof(series_information));
    test_episode->directors = (char **)malloc(2*sizeof(char *));
    test_episode->actors = (char **)malloc(2*sizeof(char *));

    test_series_info->genre = (char **)malloc(2*sizeof(char *));
    test_series_info->backdrops = (char **)malloc(2*sizeof(char *));
    
    test_episode->episode_name = "Test Episode Name";
    test_episode->episode_number = "1";
    test_episode->season_number = "1";
    test_episode->firstaired = "1970-01-01";
    test_episode->overview = "This is a test episode.";
    test_episode->directors[0] = "Test Director";
    test_episode->directors[1] = NULL;
    test_episode->actors[0] = "Test Actor";
    test_episode->actors[1] = NULL;
    test_episode->backdrop = "http://web.comhem.se/zacabeb/repository/"
    "resolution_test_1080.png";
    
    test_series_info->series_ID = "101010";
    test_series_info->series_name = "Test Series";
    test_series_info->genre[0] = "Test Genre";
    test_series_info->genre[1] = NULL;
    test_series_info->mpaa = "Test Rating";
    test_series_info->runtime = "30";
    test_series_info->backdrops[0] = "http://upload.wikimedia.org/wikipedia/"
    "commons/b/b6/PM5644-1920x1080.gif";
    test_series_info->backdrops[1] = NULL;
     
    LIBXML_TEST_VERSION

    write_episode(test_episode, test_series_info, "test.xml");
    
    free(test_episode->directors);
    free(test_episode->actors);
    free(test_episode);
    
    free(test_series_info->genre);
    free(test_series_info->backdrops);
    free(test_series_info);
     
    xmlCleanupParser();
     
    xmlMemoryDump();
    return 0;
}

#else
int main(void) {
    fprintf(stderr, "Writer or output support not compiled in\n");
    exit(1);
}*/
#endif


