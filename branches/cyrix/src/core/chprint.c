/* nccc is a chat client that provides a neat ncurses GUI and an
 * interface for extending its compatibility without programming skills.
 * Copyright (C) 2004 Alex T. and Marcel Wichern 
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation Europe e.V., Villa Vogelsang, Antonienallee 1, 
 * 45279 Essen, Germany.
*/

/* Load and use the **CHPRINT .ini-files located in chprint/ */

#include <stdio.h>	/* fprintf() */
#include <sys/types.h>
#include <dirent.h>	/* opendir(), readdir(), closedir() */
#include <string.h>	/* strcat(), strcmp(), strrchr() */
#include <errno.h>	/* perror() */

#include "iniparser.h"
#include "chprint.h"
                        
extern struct chprint chprint_data[3];


int load_chprint_files(char *chprint_dir)
{
	DIR 		*dir;
	struct dirent	*dir_list;
	int		i, index = 0;
	char		inifile[32];
	
/* open **CHPRINT directory */
	if ((dir = opendir(chprint_dir)) == NULL)
	{
		fprintf(stderr, "kann **CHPRINT-Verzeichnis \"%s\" nicht oeffnen", chprint_dir);
		return 2;
	}
/* find .ini-files and save their names */
	while ((dir_list = readdir(dir)) != NULL)
	{
		if (strcmp(strrchr(dir_list->d_name, '.'), ".ini") == 0)
		{
/* printf("%s\n", dir_list->d_name); */
			chprint_data[index++].filename = dir_list->d_name;
		}
	}
	closedir(dir);
/* load each ini-file */
	for(i = 0; i < index; i++)
	{
/* need the correct path to the .ini's (should be in the config) */
		sprintf(inifile, "chprint/");
		strcat(inifile, chprint_data[i].filename);
/* debug info */
		fprintf(stderr, "loading ini-file: %s\n", inifile);
/* load it ! */
		chprint_data[i].data = iniparser_load(inifile);
/* loading ok ? */
		if (chprint_data[i].data == NULL)
		{
			fprintf(stderr, "kann %s nicht parsen\n", chprint_data[i].filename);
			perror("Fehler beim laden der INI-Datei");
			return -1;
		}
/* show it to me, baby ;) */
		iniparser_dump(chprint_data[i].data, stderr);
	}
	
	return 0;
}
