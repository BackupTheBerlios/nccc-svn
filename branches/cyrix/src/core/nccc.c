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

/* ncurses chat client main executable */

#include "config.h"

#include <stdio.h>	/* [f]printf() */

#include "chprint.h"
#include "conf.h"

char *homedir;                          
struct chprint chprint_data[3];
                          
/* this is the root */
int main(int argc, char *argv[])
{
	int 	status;
	
/* were any command-line arguments specified? */
	if(argc >1)
	{
/* yes, print them. */
		int i;
		printf("Arguments given:\n ");
		for(i = 1; i < argc; ++i)
		{
			printf("%s\n", argv[i]);
		}
	}	

/* load configuration */
	fprintf(stderr, "Initializing configuration...\n");
	status = init_config();
	fprintf(stderr, "home-directory: %s\n", homedir);
	fprintf(stderr,"Errorcode: %i\n", status);
/* load the **CHPRINT Translation files */	
	status = load_chprint_files();
/* test if loading succeeded */		
	fprintf(stderr,"Errorcode: %i\n", status);

	
	return status;	
}
