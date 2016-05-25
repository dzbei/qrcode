/*****************************************************************************
 *
 * Copyright 2002
 * Nanotron Technologies
 *
 * Author: S. Rohdemann
 *
 * Description :
 *    This file contains the source code for the linked-list functions
 *    of the Service Library.
 *
 ****************************************************************************/

/*
 * $Log$
 */

#include "liblist.h"

/****************************************************************************/
void	ListInsertFirst(ListT **root, ListT *element)
{
	ENTER_TASK;
    element->liNext = *root;
	element->liPrev = NULL;
    
	*root = element;
    
    if (element->liNext != NULL)
	{
      (element->liNext)->liPrev = element;
    }
	LEAVE_TASK;
}



/****************************************************************************/
void	ListInsertBefore(ListT *actual, ListT *element)
{
	ENTER_TASK;
	element->liNext = actual;
	element->liPrev = actual->liPrev;

	if(actual->liPrev != NULL)
	{
		(actual->liPrev)->liNext = element;
	}
	actual->liPrev = element;
	LEAVE_TASK;
}



/****************************************************************************/
void	ListInsertAfter(ListT *actual, ListT *element)
{
	ENTER_TASK;
	element->liNext = actual->liNext;
	element->liPrev = actual;

	if(actual->liNext != NULL)
	{
		(actual->liNext)->liPrev = element;
	}
	actual->liNext = element;
	LEAVE_TASK;
}



/****************************************************************************/
void	ListDelete(ListT **root, ListT *element)
{
	ENTER_TASK;
	/* removing the first element ??? */
	if(*root == element)
	{
		*root = element->liNext;
		if(*root != NULL)
		{
			(*root)->liPrev = NULL;
		}
	}else
	{
		if(element->liNext != NULL)
		{
			(element->liNext)->liPrev = element->liPrev;
		}
		if(element->liPrev != NULL)
		{
			(element->liPrev)->liNext = element->liNext;
		}
	}
	LEAVE_TASK;
}
