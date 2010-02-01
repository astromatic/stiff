 /*
 				globals.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	Global declarations.
*
*	Last modify:	30/12/2009
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include	"types.h"

/*----------------------- miscellaneous variables ---------------------------*/
char		gstr[MAXCHAR];

/*------------------------------- functions ---------------------------------*/
extern double	counter_seconds();

extern void	makeit(void),
		write_error(char *msg1, char *msg2);

