/*
 				datamem.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	Include file for datamem.c
*
*	Last modify:	19/12/2009
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*------------------------------- functions ---------------------------------*/

extern void	free_data(float *pixbuf, size_t ndata, char *swapname);

extern int	set_maxdataram(size_t maxram),
		set_maxdatavram(size_t maxvram),
		set_dataswapdir(char *dirname);

extern float	*alloc_data(size_t ndata, char **swapname);
