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
** Module Name: rrpVector.h
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
**    RRPCloneVector(RRPVECTOR*);
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

#ifndef _RRP_VECTOR_H_
#define _RRP_VECTOR_H_

typedef struct _RRPELEMENT_NODE  RRPELEMENT_NODE;

struct _RRPELEMENT_NODE {
	char* value;
	RRPELEMENT_NODE* next;
};

typedef struct _RRPVECTOR  RRPVECTOR;

struct _RRPVECTOR {
	int count;
	RRPELEMENT_NODE* current;
	RRPELEMENT_NODE* head;
	RRPELEMENT_NODE* tail;
};

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
RRPVECTOR*  RRPCreateVector(void);

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
RRPVECTOR* RRPCloneVector(RRPVECTOR*);

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
int RRPAddVectorElement(RRPVECTOR*, char*);

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
int RRPRemoveAllVectorElements(RRPVECTOR*);

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
int RRPFreeVector(RRPVECTOR*);

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
int RRPGetVectorSize(RRPVECTOR*);

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
char* RRPGetVectorElementAt(RRPVECTOR*, int);

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
int RRPDeleteVectorElementAt(RRPVECTOR*, int);

#endif /* _RRP_VECTOR_H_ */
