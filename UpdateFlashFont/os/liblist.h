/**
 * @file liblist.h
 * @author A. Krause, S. Rohdemann
 * @date 2009-07-20
 * @c Nanotron Technologies GmbH
 * @brief Type and function definitions for the abstract datatype "double 
 * chained list".
 */

#ifndef _LIBLIST_H
#define _LIBLIST_H

#include "main.h"
/**
 * @brief Generic list type.
 */
typedef	struct	_listt_	{
	struct	_listt_		*liNext;	/**< Points to the next list element.*/
	struct	_listt_		*liPrev;	/**< Points to the previous list element.*/
	void				*liData;	/**< Points to the enlisted data.*/
}	ListT;


/**
 * @brief Inserts an element at the beginning of a list.
 * @param root The root pointer of the linked list. Points to the first element 
 * in the list.
 * @param element The element to be inserted as the first element of the list.
 */
extern	void	ListInsertFirst(ListT **root, ListT *element);



/**
 * @brief Inserts an element before another list element.
 * @param actual Actual list element.
 * @param element New element to be inserted before the @ref actual element.
 * @warning Do not use this function to insert a new element before the first 
 * list element. In this case use the function @ref ListInsertFirst , instead.
 */
extern	void	ListInsertBefore(ListT *actual, ListT *element);



/**
 * @brief Inserts an element after another list element.
 * @param actual Actual list element.
 * @param element New element to be inserted after the @ref actual element.
 */
extern	void	ListInsertAfter(ListT *actual, ListT *element);


/**
 * @brief Deletes an element from the list.
 * @param root The given list.
 * @param element The element to be deleted.
 */
extern	void	ListDelete(ListT **root, ListT *element);


#endif /*_LIBLIST_H*/
