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

/* Load, write and use the nccc.conf located in ~/.nccc */   

#include <stdio.h>	/* [f]printf() */
#include <string.h>	/* strcat() */
#include <dirent.h>	/* opendir(), readdir(), closedir() */
#include <sys/types.h>	/* uid_t */
#include <sys/stat.h>	/* mkdir() */
#include <unistd.h>	/* getuid() */
#include <pwd.h>	/* getpwuid() */
#include <stdlib.h>	/* getenv() */
#include <errno.h>	/* perror() */

#include "config.h"

extern char *homedir;

static int get_homedir();
static int create_config();

/* check for ~./nccc/nccc.conf and create one if there is none */
int init_config()
{
	DIR             *dir;
        struct dirent   *dir_list;
        char		*confdir="";

	get_homedir();
	confdir = strdup(homedir);
	strcat(confdir, "/.nccc");
/* open config directory */
	if((dir = opendir(confdir)) == NULL)
	{
/* it does not exist, create it */
        	if(mkdir(confdir, 0700) < 0)
        	{
        		perror("Fehler beim erstellen des config-Verzeichnisses");
        		return 110;
        	}
        	if((dir = opendir(confdir)) == NULL)
        	{
        		perror("Fehler beim oeffnen des config-Verzeichnisses");
        		return 111;
        	}
        }
/* it exists, now look for the config-file */
        while((dir_list = readdir(dir)) != NULL) 
        {
        	if(strcmp(dir_list->d_name, "nccc.conf") == 0)
        	{
/* config-file found ! */
        		return 0;
        	}
        }
	if(create_config() != 0)
	{
		return 112;
	}
	return 0;
}

/* retrieve home directory */
static int get_homedir()
{
	struct passwd 	*pwd;
	
	pwd = getpwuid(getuid());
	if(pwd)
	{
		homedir = pwd->pw_dir;
	} else {
		homedir = getenv("HOME");
	}
	if(!homedir)
	{
		return 100;
	}
	return 0; 	
}

/* build config-file from scratch */
static int create_config()
{
	FILE		*ini;
	
	ini = fopen("/home/cyrix/.nccc/nccc.conf", "w");
	fprintf(ini, "[INFO]\nversion = 1\n\n");
	fclose(ini);
	return 0;
}

/* verify config-file */
int check_config()
{
	return 0;
}

