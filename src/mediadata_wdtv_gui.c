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

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

static gchar *path;

gboolean delete1 (GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
   g_print("Delete Event\n");
   return FALSE;
}

void destroy1 (GObject *object, gpointer user_data)
{
    gtk_main_quit();
}

gboolean update1 (GtkWidget *entry1, GdkEventKey *event, gpointer label1)
{ 
   gtk_label_set_text(GTK_LABEL(label1), gtk_entry_get_text(GTK_ENTRY(entry1)));
   
   return TRUE;
}

gchar* determine_language_abrev(gchar* lang) {
    if (!(strcmp(lang, "English")))
        return "en";
    else if (!(strcmp(lang, "Dansk")))
        return "da";
    else if (!(strcmp(lang, "Suomeksi")))
        return "fi";
    else if (!(strcmp(lang, "Nederlands")))
        return "nl";
    else if (!(strcmp(lang, "Deutsch")))
        return "de";
    else if (!(strcmp(lang, "Italiano")))
        return "it";
    else if (!(strcmp(lang, "EspaÃƒÂ±ol")))
        return "es";
    else if (!(strcmp(lang, "FranÃƒÂ§ais")))
        return "fr";
    else if (!(strcmp(lang, "Polski")))
        return "pl";
    else if (!(strcmp(lang, "Magyar")))
        return "hu";
    else if (!(strcmp(lang, "ÃŽâ€¢ÃŽÂ»ÃŽÂ»ÃŽÂ·ÃŽÂ½ÃŽÂ¹ÃŽÂºÃŽÂ¬")))
        return "el";
    else if (!(strcmp(lang, "TÃƒÂ¼rkÃƒÂ§e")))
        return "tr";
    else if (!(strcmp(lang, "Ã‘â‚¬Ã‘Æ’Ã‘ï¿½Ã‘ï¿½Ã�ÂºÃ�Â¸Ã�Â¹ Ã‘ï¿½Ã�Â·Ã‘â€¹Ã�Âº")))
        return "ru";
    else if (!(strcmp(lang, "Ã—Â¢Ã—â€˜Ã—Â¨Ã—â„¢Ã—Âª")))
        return "he";
    else if (!(strcmp(lang, "Ã¦â€”Â¥Ã¦Å“Â¬Ã¨ÂªÅ¾")))
        return "ja";
    else if (!(strcmp(lang, "PortuguÃƒÂªs")))
        return "zh";
    else if (!(strcmp(lang, "Ã¤Â¸Â­Ã¦â€“â€¡")))
        return "pl";
    else if (!(strcmp(lang, "Ã„ï¿½eÃ…Â¡tina")))
        return "cs";
    else if (!(strcmp(lang, "Slovenski")))
        return "sl";
    else if (!(strcmp(lang, "Hrvatski")))
        return "hr";
    else if (!(strcmp(lang, "Ã­â€¢Å“ÃªÂµÂ­Ã¬â€“Â´")))
        return "ko";
    else if (!(strcmp(lang, "Svenska")))
        return "sv";
    else if (!(strcmp(lang, "Norsk")))
        return "no";
    else 
        return NULL;
}

void button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget **filters = (GtkWidget**)data;
    char* cmd = (char*)calloc(3000, sizeof(char));
    gchar *series_ID = g_strdup(gtk_entry_get_text(GTK_ENTRY(filters[0])));
    gchar *order_type = g_strdup(gtk_combo_box_text_get_active_text(
        GTK_COMBO_BOX_TEXT(filters[1])));
    gchar *lang = g_strdup(gtk_combo_box_text_get_active_text(
        GTK_COMBO_BOX_TEXT(filters[2])));
    gchar *season_num = g_strdup(gtk_entry_get_text(GTK_ENTRY(filters[3])));
    gchar *episode_num = g_strdup(gtk_entry_get_text(GTK_ENTRY(filters[4])));
    gboolean complete = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(filters[5]));
    gchar *format = g_strdup(gtk_entry_get_text(GTK_ENTRY(filters[6])));
    gchar *abrev = determine_language_abrev(lang);
    gchar *dvd_order = !(strcmp(order_type,"DVD Order")) ? "-d" : "";

    char* cwd = "./";

    #if defined(__MINGW32__) || defined(WIN32)
    cwd = "";
    #endif

    if (!(strcmp(path,""))) {
        sprintf(path, "./");
        #if defined(__MINGW32__) || defined(WIN32)
        sprintf(path, "%CD%");
        #endif
    }

    if (!(strcmp(format, ""))) {
        g_free(format);
        format = (gchar*)calloc(10,sizeof(gchar));
        sprintf(format,"+T.+^S+^E");
    }
    
    if (strcmp(season_num,"") && strcmp(episode_num,"") && !complete) {
        sprintf(cmd, "%smediadata_wdtv -i %s %s -l %s -s %s -e %s -f %s -o %s",
                cwd, series_ID, dvd_order, abrev, season_num, episode_num,
                format, path);
        
    } else if (strcmp(season_num,"") && !(strcmp(episode_num,"")) && 
        !complete) {
        sprintf(cmd, "%smediadata_wdtv -i %s %s -l %s -s %s -o %s -f %s",
            cwd, series_ID, dvd_order, abrev, season_num, path, format);
    } else if (strcmp(season_num,"") && strcmp(episode_num,"") && complete) {
        sprintf(cmd, 
            "%smediadata_wdtv -i %s %s -l %s -s %s -e %s -c -o %s -f %s", cwd,
            series_ID, dvd_order, abrev, season_num, episode_num, path, format);
    } else if (season_num && !(strcmp(episode_num,"")) && complete) {
        sprintf(cmd, "%smediadata_wdtv -i %s %s -l %s -s %s -c -o %s -f %s",
            cwd, series_ID, dvd_order, abrev, season_num, path, format);
    } else {
        sprintf(cmd, "%smediadata_wdtv -i %s %s -l %s -o %s -f %s",
            cwd, series_ID, dvd_order, abrev, path, format);
    }

    g_print("%s\n", cmd);

    system(cmd);
    free(cmd);
    g_free(series_ID);
    g_free(order_type);
    g_free(lang);
    g_free(season_num);
    g_free(episode_num);
    g_free(format);
}

void path_changed(GtkWidget *widget, gpointer data) 
{
    path = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(widget));


    g_print("path: %s\n", path);

}

int main(int argc, char **argv)
{
    GtkBuilder *mediadata;
    GtkWidget **filters = (GtkWidget**)malloc(7*sizeof(GtkWidget*));
    GtkWidget *button, *window;
    GError *result = NULL;

    gtk_init(&argc, &argv);

    mediadata = gtk_builder_new();

    path = (gchar*)calloc(2048,sizeof(gchar));

    gtk_builder_add_from_file (mediadata, "mediadata.glade", &result);

    if (result!=NULL) {
        free(filters);
        g_free(path);
        g_print("Error: %s\n", result->message);
        exit(EXIT_FAILURE);
    }

    button = (GtkWidget*)gtk_builder_get_object(mediadata,
        "path_button");
    filters[0] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "series_ID_entry");
    filters[1] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "order_type_combo_box");
    filters[2] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "lang_combo_box");
    filters[3] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "season_num_entry");
    filters[4] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "episode_num_entry");
    filters[5] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "complete_check_button");
    filters[6] = (GtkWidget*)gtk_builder_get_object(mediadata,
        "format_entry");

    /* Set default value for combo boxes */
    gtk_combo_box_set_active(GTK_COMBO_BOX(filters[1]), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(filters[2]), 0);

    g_signal_connect(G_OBJECT(button), "clicked",
        G_CALLBACK(button_clicked), filters);

    window = GTK_WIDGET (gtk_builder_get_object (mediadata, "mediadata_gui"));

    gtk_builder_connect_signals(mediadata, NULL);

    g_object_unref (G_OBJECT (mediadata));

    gtk_widget_show (window);

    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete1),
        NULL);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy1), NULL);

    gtk_main();

    free(filters);
    g_free(path);

    exit(EXIT_SUCCESS);

}
