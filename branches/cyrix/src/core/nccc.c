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

/* ncurses chat client main executable */

#include <stdio.h>

#include "chprint.h"
                          
struct chprint chprint_data[3];
                          

int main(int argc, char *argv[])
{
	int 	status;
	
/* Were any command-line arguments specified? */
	if (argc >1)
	{
/* Yes, print them. */
		int i;
		printf ("Arguments given:\n ");
		for (i = 1; i < argc; ++i)
		{
			printf("%s\n", argv[i]);
		}
	}	

/* load the **CHPRINT Translation files */	
	status = load_chprint_files("chprint");
/* test if loading succeeded */		
	printf ("test filename #1 of chprint_data struct: %s\n", chprint_data[0].filename);
/* test if inifile is usable */
	if (check_chprint_file(0) == 0)
	{
		printf("INI-File %s entspricht den **CHPRINT-Spezifikationen v0.1\n",chprint_data[0].filename);
	} else {	
		printf("INI-File %s entspricht NICHT den **CHPRINT-Spezifikationen v0.1\n",chprint_data[0].filename);
	}
	
	return status;
}
