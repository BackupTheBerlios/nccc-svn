/* nccc is a chat client that provides a neat ncurses GUI and an
 * interface for extending its compatibility without programming skills.
 * Copyright (C) 2004 by Alex T.
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

/* **CHatPRotocolINTerface-Implementation */

#include "config.h"

#include <stdio.h>	/* fprintf() */
#include <sys/types.h>
#include <dirent.h>	/* opendir(), readdir(), closedir() */
#include <string.h>	/* strcpy(), strcat(), strcmp(), strrchr(), strlen() */
#include <stdlib.h> 	/* malloc(), realloc() */
#include <errno.h>	/* perror() */

#include "iniparser.h"
#include "chprint.h"
#include "path.h"

extern struct chprint chprint_data[3];

/* Load the **CHPRINT .ini-files located in chprint/ */
int load_chprint_files()
{
	DIR 		*dir;
	struct dirent	*dir_list;
	int		i, index = 0, status = 0;
	char		*chprintdir, tmp[80];
	
/* search and open **CHPRINT directory */
	if((dir = opendir(gdatadir)) == NULL)
	{
		if((dir = opendir(ldatadir)) == NULL)
		{
			fprintf(stderr, "no datadir supplied. trying parent dir...\n");
			strcpy(tmp, "..");
			if((dir = opendir(tmp)) == NULL)
			{
				fprintf(stderr, "parent dir not accessible. exiting...\n");
				return 200;
			}
		} else {
			strcpy(tmp, ldatadir);
		}
	} else {
		strcpy(tmp, gdatadir);
	}
	closedir(dir);
	strcat(tmp, "/chprint");
	chprintdir = malloc(strlen(tmp)+1); 
	strcpy(chprintdir, tmp);
	fprintf(stderr, "found chprint-dir: %s\n", chprintdir);

	if((dir = opendir(chprintdir)) == NULL)
	{
		perror("fehlermeldung");
		fprintf(stderr, "chprint dir not accessible. exiting...\n");
		return 200;
	}
	
/* find .ini-files and save their names */
	while(NULL != (dir_list = readdir(dir)))
	{
		if(strrchr(dir_list->d_name, '.')) /* without this line filenames w/o '.' will cause a segfault */
		{
			if(strcmp(strrchr(dir_list->d_name, '.'), ".ini") == 0)
			{
				chprint_data[index].filename = dir_list->d_name;
				index++;
				/*fprintf(stderr, "%s is an .ini-file..\n", dir_list->d_name);*/
			}
		}
	}
	closedir(dir);	
	
/* load each ini-file */
	for(i = 0; i < index; i++)
	{
		sprintf(tmp, "%s/%s", chprintdir, chprint_data[i].filename);
/* debug info */
		fprintf(stderr, "loading ini-file: %s\n", tmp);
/* load it ! */
		chprint_data[i].data = iniparser_load(tmp);
/* loading ok ? */
		if(chprint_data[i].data == NULL)
		{
			return 201;
		}
		status = check_chprint_file(i);
		if(status)
		{
			fprintf(stderr, "ini-file %s not ok: Errorcode %i\n", chprint_data[i].filename, status);			
		} else {
			fprintf(stderr, "ini-file %s ok! **CHPRINT-Version: %i\n", chprint_data[i].filename, chprint_data[i].version);
		}
/* debug: show it to me, baby ;) */
/*		iniparser_dump(chprint_data[i].data, stderr); */
	}			
	return 0;
}

/* check a **CHPRINT-file for completeness */
int check_chprint_file(int index)
{
	char	*chprint_cmdset_v01[6] = {"LOGIN", "CHROOM", "LOGOUT", "SENDMSG", "RECVERR", "KEEPALIVE"};
	int	i;
	char	chprint_cmd[15];
	
/* check INFO-Section */
	if(iniparser_find_entry(chprint_data[index].data, "INFO") == 0)
	{
		return 210;
	} else {		
		if(iniparser_getstring(chprint_data[index].data, "INFO:Name", "NULL") == "NULL")
		{
			return 211;
		}
		if(iniparser_getstring(chprint_data[index].data, "INFO:LongName", "NULL") == "NULL")
		{
			return 211;
		}
	}
/* check CMD-Section for **CHPRINT Version 0.1 commands */	
	if(iniparser_find_entry(chprint_data[index].data, "CMD") == 0)
	{
		return 212;
	} else {		
		for(i = 0; i < 6; i++)
		{
			sprintf(chprint_cmd, "CMD:");
			strcat(chprint_cmd, chprint_cmdset_v01[i]);
			if(iniparser_getstring(chprint_data[index].data, chprint_cmd, "NULL") == "NULL")
			{
				return 213;
			}
/* check CMD-Section for Subsections */
			if(strcmp(iniparser_getstring(chprint_data[index].data, chprint_cmd, "NULL"), "SUB") == 0)
			{
				sprintf(chprint_cmd, "CMD/");
				strcat(chprint_cmd, chprint_cmdset_v01[i]);
				if(iniparser_find_entry(chprint_data[index].data, chprint_cmd) == 0)
				{
					return 214;
				}
			}
		}
	}
	chprint_data[index].version = 1;
	return 0;
}
