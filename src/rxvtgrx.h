/*--------------------------------*-C-*---------------------------------*
 * File:	rxvtgrx.h
 *
 * Stuff for text alignment for rxvt special graphics mode
 *
 * alignment
 * Top:
 *	text is placed so that the specified point is at the top of the
 *	capital letters
 * Center:
 *	text is placed so that the specified point is equidistant from the
 *	bottom of descenders and the top of the capital letters
 * Bottom:
 *	text is placed so that the bottom of descenders is on the specified
 *	point
 * Base:
 *	text is placed so that the bottom of the characters with no descenders
 *	is on the specified point
 * Caps_Center:
 *	text is placed so that the specified point is equidistant from the
 *	bottom and tops of capital letters
 *----------------------------------------------------------------------*/
#ifndef RXVTGRX_H_
#define RXVTGRX_H_

#define GRX_SCALE		10000

#define RIGHT_TEXT		0x10
#define HCENTER_TEXT		0x20
#define LEFT_TEXT		0x30
#define HORIZONTAL_ALIGNMENT	0x70

#define TOP_TEXT		0x01
#define VCENTER_TEXT		0x02
#define BOTTOM_TEXT		0x03
#define BASE_TEXT		0x04
#define VCAPS_CENTER_TEXT	0x05
#define VERTICAL_ALIGNMENT	0x0F

#if 0	/* this would be nicer */
# define TXT_RIGHT		'r'
# define TXT_CENTER		'c'
# define TXT_LEFT		'l'

# define TXT_TOP		't'
# define TXT_VCENTER		'v'
# define TXT_BOTTOM		'b'
# define TXT_BASE		'_'
# define TXT_VCAPS_CENTER	'C'
#endif

#endif /* _RXVTGRX_H_ */
/*----------------------- end-of-file (C header) -----------------------*/
