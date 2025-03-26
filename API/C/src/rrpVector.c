/* ===========================================================================
 * Copyright (C) 2000 VeriSign, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * VeriSign Global Registry Service
 * 21345 Ridgetop Circle
 * Dulles, VA 20166
 * ===========================================================================
 * The RRP, APIs and Software are provided "as-is" and without any warranty
 * of any kind.  NSI EXPRESSLY DISCLAIMS ALL WARRANTIES AND/OR CONDITIONS,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * AND CONDITIONS OF MERCHANTABILITY OR SATISFACTORY QUALITY AND FITNESS FOR
 * A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  NSI DOES
 * NOT WARRANT THAT THE FUNCTIONS CONTAINED IN THE RRP, APIs OR SOFTWARE
 * WILL MEET REGISTRAR'S REQUIREMENTS, OR THAT THE OPERATION OF THE RRP,
 * APIs OR SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE,OR THAT DEFECTS IN
 * THE RRP, APIs OR SOFTWARE WILL BE CORRECTED.  FURTHERMORE, NSI DOES NOT
 * WARRANT NOR MAKE ANY REPRESENTATIONS REGARDING THE USE OR THE RESULTS OF
 * THE RRP, APIs, SOFTWARE OR RELATED DOCUMENTATION IN TERMS OF THEIR
 * CORRECTNESS, ACCURACY, RELIABILITY, OR OTHERWISE.  SHOULD THE RRP, APIs
 * OR SOFTWARE PROVE DEFECTIVE, REGISTRAR ASSUMES THE ENTIRE COST OF ALL
 * NECESSARY SERVICING, REPAIR OR CORRECTION.
 * ======================================================================== */

/*
**
** Module Name: rrpVector.c
**
** Programmer: Stephen Mahlstedt
** Version: 1.0 for RRP version 1.0
**
** Date: 01/13/1999
**
** Description: rrpVector provides an API to a ordered linked-list
**              structure. Each node (element) in the linked-list
**              contains a string value
**
**              In the event of an internal error (bad parameter, memory
**              allocation error, etc.) each function will return a
**              value indicating that an error has occured (see function
**              descriptions below). An internal error code that
**              identifies the error will be set. The error code can
**              be accessed and interpreted by the functions defined in
**              rrpInternalError.h (see API documentation)
**
** Entry Points:
**
** 	RRPCreateVector(void);
** 	RRPAddVectorElement(RRPVECTOR*, char*);
** 	RRPRemoveAllVectorElements(RRPVECTOR*);
** 	RRPFreeVector(RRPVECTOR*);
** 	RRPGetVectorSize(RRPVECTOR*);
** 	RRPGetVectorElementAt(RRPVECTOR*, int);
** 	RRPDeleteVectorElementAt(RRPVECTOR*, int);
**
** Changes:
**
*/


#include <stdlib.h>
#include <string.h>
#include "rrpVector.h"
#include "rrpInternalError.h"



/*
**
** Function: RRPCreateVector
**
** Description: Allocates memory for and returns a pointer to an
**              RRPVECTOR structure
**
** Input: none
**
** Output: none
**
** Return: RRPVECTOR* - a pointer to an allocated RRPVECTOR structure
**         NULL is returned if an internal error occurs.
**
** Note:   THE POINTER RETURNED MUST BE PASSED TO RRPFreeVector() IN ORDER
**         TO FREE THE MEMORY ALLOCATED UPON CREATION OF STRUCTURE.
*/

RRPVECTOR*
RRPCreateVector () {

	RRPVECTOR* newVector = NULL;

	/*
	** Allocate memory for new RRPVECTOR structure
	*/
	newVector = (RRPVECTOR*) calloc(1, sizeof(RRPVECTOR));

	/*
	** Make sure that memory was allocated properly
	*/
	if (newVector == NULL) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return NULL;
	}

	newVector->count = 0;
	newVector->current = NULL;
	newVector->head = NULL;
	newVector->tail = NULL;

	return newVector;

} /* RRPCreateVector */






/*
**
** Function: RRPCloneVector
**
** Description: Allocates memory for and returns a pointer to a
**              a new RRPVECTOR structure which is an indentical
**              copy of another
**
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure to clone
**
** Output: none
**
** Return: RRPVECTOR* - a pointer to a new allocated RRPVECTOR structure.
**         NULL is returned if an internal error occurs.
**
** Note:   THE POINTER RETURNED MUST BE PASSED TO RRPFreeVector() IN ORDER
**         TO FREE THE MEMORY ALLOCATED UPON CREATION OF STRUCTURE.
*/

RRPVECTOR*
RRPCloneVector (
	RRPVECTOR* oldVector
) {
	RRPVECTOR* newVector = NULL;
	RRPELEMENT_NODE* newNode  = NULL;
	RRPELEMENT_NODE* lastNode = NULL;
	RRPELEMENT_NODE* currentNode  = NULL;

	/*
	** Validate parameters
	*/
	if (oldVector == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	/*
	** Create new empty RRPVECTOR structure
	*/
	newVector = RRPCreateVector();

	if (newVector == NULL) {
		/*
		** just return NULL because RRPCreateVector() should have
		** already set error code
		*/
		return NULL;
	}


	/*
	** If there are no elements in the old RRPVECTOR struture
	** then just return the new structure "as is"
	*/
	if (oldVector->count == 0) {
		return newVector;
	}


	currentNode = oldVector->head;

	while (currentNode != NULL) {
		newNode = (RRPELEMENT_NODE*) calloc(1, sizeof(RRPELEMENT_NODE));

		/*
		** Check to make sure memory was properly allocated
		*/
		if (newNode == NULL) {
			RRPFreeVector(newVector);
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}

		newVector->count++;

		newNode->value = strdup(currentNode->value);

		/*
		** Check to make sure memory was properly allocated
		*/
		if (newNode->value == NULL) {
			RRPFreeVector(newVector);
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}

		if (lastNode != NULL) {
			lastNode->next = newNode;
		}
		else {
			newVector->head = newNode;
		}

		if (oldVector->current == currentNode) {
			newVector->current = newNode;
		}
		lastNode = newNode;
		currentNode = currentNode->next;

		if (currentNode == NULL) {
			newVector->tail = newNode;
			newNode->next = NULL;
		}
	}
	return newVector;



} /* RRPCloneVector */






/*
**
** Function: RRPAddVectorElement
**
** Description: Adds a new vector element to the end of an
**              RRPVECTOR structure
**
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure
**        char* - the element string
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs.
**
**
*/

int
RRPAddVectorElement (
	RRPVECTOR* vector,
	char* value
) {
	RRPELEMENT_NODE* lastNode = NULL;
	RRPELEMENT_NODE* newNode = NULL;

	/*
	** Validate parameters
	*/
	if (vector == NULL || value == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	/*
	** Allocate memory for new RRPELEMENT_NODE structure
	*/
	newNode = (RRPELEMENT_NODE*)calloc(1, sizeof(RRPELEMENT_NODE));	

	/*
	** Verify that memory was allocated properly
	*/
	if (newNode == NULL) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}

	/*
	** Allocate memory for node string value
	*/
	newNode->value = strdup(value);

	/*
	** Verify that memory was allocated properly
	*/
	if (newNode->value == NULL) {
		free(newNode);
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}

	newNode->next = NULL;

	if (vector->count == 0) {
		vector->head = newNode;
		vector->current = newNode;
	}
	else {
		lastNode = vector->tail;
		lastNode->next = newNode;
	}
	vector->tail = newNode;
	vector->count++;

	return 0;
	
} /* RRPAddVectorElement */








/*
**
** Function: RRPRemoveAllVectorElements
**
** Description: Removes all of the elements in an RRPVECTOR structure
**
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs.
**
**
*/

int
RRPRemoveAllVectorElements (
	RRPVECTOR* vector
) {
   RRPELEMENT_NODE* currentNode;
   RRPELEMENT_NODE* nextNode;

   /*
   ** Validate parameters
   */
   if (vector == NULL) {
      RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
      return -1;
   }

   currentNode = vector->head;

   while (currentNode != NULL) {
      nextNode = currentNode->next;

      free(currentNode->value);
      free(currentNode);

      currentNode = nextNode;
   }

   vector->count = 0;
   vector->head = NULL;
   vector->tail = NULL;
   vector->current = NULL;

   return 0;

} /* RRPRemoveAllVectorElements */






/*
**
** Function: RRPFreeVector
**
** Description: Frees all memory allocated for a specific RRPVECTOR
**              structure
**
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs.
**
**
*/

int
RRPFreeVector (
	RRPVECTOR* vector
) {

	/*
	** Validate parameters
	*/
	if (vector == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	if (RRPRemoveAllVectorElements(vector) < 0) {
		return -1;
	}

	free(vector);
	return 0;

} /* RRPFreeVector */







/*
**
** Function: RRPGetVectorSize
**
** Description: Returns number of elements (nodes) in an RRPVECTOR
**              structure
**
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure
**
** Output: none
**
** Return: int - returns number of vector elements if successful.
**               Returns -1 if an internal error occurs.
**
**
*/

int
RRPGetVectorSize (
	RRPVECTOR* vector
) {
	/*
	** Validate parameters
	*/
	if (vector == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	return vector->count;

} /* RRPGetVectorSize */






/*
**
** Function: RRPGetVectorElementAt
**
** Description: Returns the value of the element at the specified index
**              
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure
**        int - element index
**
** Output: none
**
** Return: char* - returns element value if vector index exists. Returns NULL
**                 if an internal error occurs
**
**
*/

char*
RRPGetVectorElementAt (
	RRPVECTOR* vector,
	int index
) {
	int position = 0;
	RRPELEMENT_NODE* node = NULL;

	/*
	** Validate parameters
	*/
	if (vector == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}
	else if (index < 0 || index >= vector->count) {
		RRPSetInternalErrorCode(RRP_INVALID_VECTOR_INDEX_ERROR);
		return NULL;
	}

	node = vector->head;

	for (position = 0; position < vector->count; position++) {
		if (position == index) {
			return node->value;
		}
		node = node->next;
	}

	return NULL;

} /* RRPGetVectorElementAt */






/*
**
** Function: RRPDeleteVectorElementAt
**
** Description: Deletes the element at the specified index
**              
** Input: RRPVECTOR* - a pointer to an RRPVECTOR structure
**        int - index of the element to delete
**
** Output: none
**
** Return: int - returns 0 is successful. Returns -1 if an internal
**         error occurs
**
**
*/

int
RRPDeleteVectorElementAt (
	RRPVECTOR* vector,
	int index
) {

	int position = 0;
	RRPELEMENT_NODE* currentNode = NULL;
	RRPELEMENT_NODE* previousNode = NULL;

	/*
	** Validate parameters
	*/
	if (vector == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}
	else if (index < 0 || index >= vector->count) {
		RRPSetInternalErrorCode(RRP_INVALID_VECTOR_INDEX_ERROR);
		return -1;
	}

	currentNode = vector->head;
	previousNode = vector->head;

	for (position = 0; position < vector->count; position++) {
		if (position == index) {
			if (currentNode == vector->head) {
				vector->head = currentNode->next;
			}
			else {
				previousNode->next = currentNode->next;
			}

			if (currentNode == vector->current) {
				vector->current = currentNode->next;
			}
			vector->count--;
			free(currentNode->value);
			free(currentNode);
			return 0;
		}

		previousNode = currentNode;
		currentNode = currentNode->next;
	}

	return -1;

} /* RRPDeleteVectorElementAt */



