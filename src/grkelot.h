/*
 * File:	grkelot.h
 *
 * Synopsis:	string -> greek ELOT928 string; 4-state FSM.
 *
 * Copyright (c) 1994 Angelo Haritsis. All rights reserved.
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GRKELOT_H
#define GRKELOT_H

#define GREEK_ELOT928	0
#define GREEK_IBM437	1

#ifdef __cplusplus
extern "C" {
#endif
   extern void	greek_init (void);
   extern void	greek_end (void);
   extern void	greek_reset (void);
   extern void  greek_setmode (int greek_mode);
   extern int   greek_getmode (void);
   extern int	greek_xlat (char *s, int num_chars);
#ifdef __cplusplus
}
#endif
#endif	/* _GRKELOT_H */
