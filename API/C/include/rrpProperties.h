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
** Module Name: rrpProperties.h
**
** Programmer: Stephen Mahlstedt
** Version: 1.0 for RRP version 1.0
**
** Date: 01/13/1999
**
** Description: rrpProperties provides an API to a list of properties.
**              Each property is an RRPVector structure containing one
**            	 or more string values for that property. A property is
**              identified by a unique key. There can be duplicate values
**              for one key (hence the RRPVector), but there can not be
**              any duplicate keys. Everytime a new property is added
**              to the list for the same key, the value is just added to
**              the RRPVector for that property.
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
** 	RRPCreateProperties(void);
** 	RRPCloneProperties(RRPPROPERTIES*);
** 	RRPContainsProperty(RRPPROPERTIES*, char*);
** 	RRPRemoveProperty(RRPPROPERTIES*, char*);
** 	RRPPutProperty(RRPPROPERTIES*, char*, char*);
** 	RRPPutPropertyPair(RRPPROPERTIES*, char*);
** 	RRPClearProperties(RRPPROPERTIES*);
** 	RRPFreeProperties(RRPPROPERTIES*);
** 	RRPResetPropertyPointer(RRPPROPERTIES*);
** 	RRPGetPropertiesSize(RRPPROPERTIES*);
** 	RRPGetProperty(RRPPROPERTIES*, char*);
** 	RRPGetNextPropertyKey(RRPPROPERTIES*);
**
** Changes:
**
*/
#ifndef _RRP_PROPERTIES_H_
#define _RRP_PROPERTIES_H_

#include "rrpVector.h"

typedef struct _RRPPROPERTIES  RRPPROPERTIES;
typedef struct _RRPPROPERTY_NODE  RRPPROPERTY_NODE;


#ifndef _RRP_BOOLEAN_
	#define _RRP_BOOLEAN_
	typedef enum { RRPFALSE, RRPTRUE }  RRPBOOLEAN;
#endif

struct _RRPPROPERTY_NODE {
	char* key;
	RRPVECTOR* values;
	RRPPROPERTY_NODE* next;
};

struct _RRPPROPERTIES {
	int count;
	RRPPROPERTY_NODE* current;
	RRPPROPERTY_NODE* head;
	RRPPROPERTY_NODE* tail;
};

/*
**
** Function: RRPCreateProperties
**
** Description: Allocates memory for and returns a pointer to a
**              RRPPROPERTIES structure
**
** Input: none
**
** Output: none
**
** Return: RRPPROPERTIES* - a pointer to an allocated RRPPROPERTIES
**         structure. NULL is returned if an internal error occurs
**
** Note:   THE POINTER RETURNED MUST BE PASSED TO RRPFreeProperties()
**         IN ORDER TO FREE MEMORY ALLOCATED UPON CREATION OF STRUCTURE.
**
*/
RRPPROPERTIES*  RRPCreateProperties(void);

/*
**
** Function: RRPCloneProperties
**
** Description: Allocates memory for and returns an exact copy of
**              an RRPPROPERTIES structure
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**
** Output: none
**
** Return: RRPPROPERTIES* - a pointer to a new allocated RRPPROPERTIES
**         structure. NULL is returned if an internal error occurs
**
** Note:   THE POINTER RETURNED MUST BE PASSED TO RRPFreeProperties()
**         IN ORDER TO FREE MEMORY ALLOCATED UPON CREATION OF STRUCTURE.
**
**
*/
RRPPROPERTIES* RRPCloneProperties(RRPPROPERTIES*);

/*
**
** Function: RRPContainsProperty
**
** Description: Determines whether or not a specific property exists
**              in RRPPROPERTIES structure
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**        char* - a property key which identifies the property
**
** Output: none
**
** Return: RRPBOOLEAN - returns RRPTRUE if property exists.
**                      returns RRPFALSE if property does not exist
**
**
*/
RRPBOOLEAN RRPContainsProperty(RRPPROPERTIES*, char*);

/*
**
** Function: RRPRemoveProperty
**
** Description: Removes a specific property from an RRPPROPERTIES structure
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**        char* - a property key which indentifies the property to remove
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs
**
**
*/
int RRPRemoveProperty(RRPPROPERTIES*, char*);

/*
**
** Function: RRPPutProperty
**
** Description: Adds a new property to the RRPPROPERTIES structure.
**              If the key indentifying the property does not yet exist,
**              then a new RRPVector is created and the property value
**              is added to it. If the key already exists, the property
**              value is simply added to the RRPVector structure that is
**              already associated with that property.
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**        char* - a key identifying property
**        char* - a property value
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs
**
**
*/
int RRPPutProperty(RRPPROPERTIES*, char*, char*);

/*
**
** Function: RRPPutPropertyPair
**
** Description: Same as RRPPutProperty() but the key and value are
**              derived from a single string in the format "key:value"
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**        char* - a string consisting of a key followed by a colon (':')
**                followed by a value
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs
**
**
*/
int RRPPutPropertyPair(RRPPROPERTIES*, char*);

/*
**
** Function: RRPClearProperties
**
** Description: Removes all of the properties in an RRPPROPERTIES structure.
**              All of the memory allocated for each property is released;
**              the actual RRPPROPERTIES structure itself is left intact
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs
**
**
*/
int RRPClearProperties(RRPPROPERTIES*);

/*
**
** Function: RRPFreeProperties
**
** Description: Frees all memory allocated for a specific RRPPROPERTIES
**              structure and all of it's properties 
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs
**
**
*/
int RRPFreeProperties(RRPPROPERTIES*);

/*
**
** Function: RRPResetPropertyPointer
**
** Description: Sets the current property pointer to the beginning of list
**              of properties. See RRPGetNextPropertyKey() description
**              for more about the "current property" pointer
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**
** Output: none
**
** Return: int - returns 0 if successful. Returns -1 if an internal
**               error occurs
**
**
*/
int RRPResetPropertyPointer(RRPPROPERTIES*);

/*
**
** Function: RRPGetPropertiesSize
**
** Description: Returns the number of property keys in an RRPPROPERTIES
**              structure
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**
** Output: none
**
** Return: int - returns the number of keys if successful. Returns -1
**               if an internal error occurs
**
**
*/
int RRPGetPropertiesSize(RRPPROPERTIES*);

/*
**
** Function: RRPGetProperty
**
** Description: Returns an RRPVector structure containing the values
**              for a particular property. The property is identified
**              by a specified key.
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**        char* - a key which identifies a property
**
** Output: none
**
** Return: RRPVECTOR* - a list containing one or more values for the property.
**                      Returns NULL if an internal error occurs
**
**
*/
RRPVECTOR* RRPGetProperty(RRPPROPERTIES*, char*);

/*
**
** Function: RRPGetNextPropertyKey
**
** Description: Returns the key which identifies the current property
**              and moves the current property pointer to the next property in
**              the list. If the current property pointer reaches the end of
**              of the list, then NULL will be returned. The current
**              property pointer can be reset (set to point to the first
**              property in the list) by calling RRPResetProperties()
**
** Input: RRPPROPERTIES* - a pointer to an RRPPROPERTIES structure
**
** Output: none
**
** Return: char* - the current property key. Returns NULL if the end of the
**                 properties list has been reached
**
**
*/
char* RRPGetNextPropertyKey(RRPPROPERTIES*);

#endif /* _RRP_PROPERTIES_H_ */
