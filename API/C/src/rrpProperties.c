/* ===========================================================================
 * Copyright (C) 2000 Network Solutions, Inc.
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
 * Network Solutions, Inc. Registry
 * 505 Huntmar Park Dr.
 * Herndon, VA 20170
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
**              or more string values for that property. A property is
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
**    RRPCreateProperties(void);
**    RRPCloneProperties(RRPPROPERTIES*);
**    RRPContainsProperty(RRPPROPERTIES*, char*);
**    RRPRemoveProperty(RRPPROPERTIES*, char*);
**    RRPPutProperty(RRPPROPERTIES*, char*, char*);
**    RRPPutPropertyPair(RRPPROPERTIES*, char*);
**    RRPClearProperties(RRPPROPERTIES*);
**    RRPFreeProperties(RRPPROPERTIES*);
**    RRPResetPropertyPointer(RRPPROPERTIES*);
**    RRPGetPropertiesSize(RRPPROPERTIES*);
**    RRPGetProperty(RRPPROPERTIES*, char*);
**    RRPGetNextPropertyKey(RRPPROPERTIES*);
**
** Changes:
**
**
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rrpProperties.h"
#include "rrpVector.h"
#include "rrpInternalError.h"



/*
** Function used internally to locate a propery based on it's
** key
*/
RRPPROPERTY_NODE* RRPFindProperty (RRPPROPERTIES*, char*);




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

RRPPROPERTIES*
RRPCreateProperties () {
	RRPPROPERTIES* p;

	/*
	** Allocate memory for new RRPPROPERTIES structure
	*/
	p = (RRPPROPERTIES*) calloc(1, sizeof(RRPPROPERTIES));

	/*
	** Make sure that memory was allocated properly
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return NULL;
	}

	p->count = 0;
	p->current = NULL;
	p->head = NULL;
	p->tail = NULL;

	return p;

} /* RRPCreateProperties */







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

RRPPROPERTIES*
RRPCloneProperties (
	RRPPROPERTIES* p
) {
	RRPPROPERTIES* newProperties = NULL;
	RRPPROPERTY_NODE* newNode  = NULL;
	RRPPROPERTY_NODE* last = NULL;
	RRPPROPERTY_NODE* ptr  = NULL;

	/*
	** Validate parameters
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	/*
	** Create new empty RRPPROPERTIES structure
	*/	
	newProperties = RRPCreateProperties();

	if (newProperties == NULL) {
		/*
		** just return NULL because RRPCreateProperties() should have
		** already set error code
		*/
		return NULL;
	}


	/*
	** If there are no properties in the old RRPPROPERTIES struture
	** then just return the new structure "as is"
	*/
	if (p->count == 0) {
		return newProperties;
	}


	ptr = p->head;

	while(ptr != NULL) {
		newNode = (RRPPROPERTY_NODE*) calloc(1, sizeof(RRPPROPERTY_NODE));

		/*
		** Check to make sure memory was properly allocated
		*/
		if (newNode == NULL) {
			RRPFreeProperties(newProperties);
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}

		newProperties->count++;
		
		newNode->key = strdup(ptr->key);
		newNode->values = RRPCloneVector(ptr->values);

		/*
		** Check to make sure memory was properly allocated
		*/
		if (newNode->key == NULL || newNode->values == NULL) {
			RRPFreeProperties(newProperties);
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}

		if (NULL != last) {
			last->next = newNode;
		} else {
			newProperties->head = newNode;
		}

		if (p->current == ptr) {
			newProperties->current = newNode;
		}
		last = newNode;
		ptr = ptr->next;

		if (ptr == NULL) {
			newProperties->tail = newNode;
			newNode->next = NULL;
		}
	}
	return newProperties;
}






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

int
RRPPutProperty (
	RRPPROPERTIES* p,
	char* key,
	char* value
) {
	RRPPROPERTY_NODE* oldNode = NULL;
	RRPPROPERTY_NODE* newNode = NULL;
	

	/*
	** Validate parameters
	*/
	if (p == NULL || key == NULL || value == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	/*
	**
	** If a property already exists with the same key, then
	** free old key and value and assign new key and value.
	**
	*/
	oldNode = RRPFindProperty(p, key);

	if (oldNode != NULL) {
		if (RRPAddVectorElement(oldNode->values, value) < 0) {
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return -1;
		}

		return 0;
	}

	newNode = (RRPPROPERTY_NODE*)calloc(1, sizeof(RRPPROPERTY_NODE));
	if (newNode == NULL) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}

	newNode->next = NULL;
	newNode->key = strdup(key);

	if (newNode->key == NULL) {
		free(newNode);
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}


	newNode->values = RRPCreateVector();

	if (newNode->values == NULL) {
		free(newNode->key);
		free(newNode);
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}

	if (RRPAddVectorElement(newNode->values, value) < 0) {
		RRPFreeVector(newNode->values);
		free(newNode->key);
		free(newNode);
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}

	if (p->count == 0) {
		p->head = newNode;
		p->current = newNode;
	} else {
		p->tail->next = newNode;
	}

	p->tail = newNode;
	p->count++;

	return 0;

} /* RRPPutProperty */
	
	






void
RRPDisplayProperties (
	RRPPROPERTIES* p
) {
	RRPPROPERTY_NODE* temp;
	RRPVECTOR* vector = NULL;
	int vectorSize = 0;
	int i = 0;

	/*
	** Validate parameters
	*/
	if (p == NULL) {
		return;
	}

	temp = p->head;

	while (temp) {
		vector = temp->values;
		vectorSize = RRPGetVectorSize(vector);

		for (i=0; i < vectorSize; i++) {
			printf("KEY: %s VALUE: %s\n", temp->key,
				RRPGetVectorElementAt(vector, i));
		}
		temp = temp->next;
	}
} /* RRPDisplayProperties */






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

int
RRPFreeProperties (
	RRPPROPERTIES* p
) {

	/*
	** Validate parameters
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	if (RRPClearProperties(p) < 0) {
		return -1;
	}
	free(p);
	return 0;

} /* RRPFreeProperties */







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

int
RRPClearProperties (
	RRPPROPERTIES* p
) {
	RRPPROPERTY_NODE* temp;
	RRPPROPERTY_NODE* next;

	/*
	** Validate parameters
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	temp = p->head;
	
	while (temp) {
		next = temp->next;

		free(temp->key);	
		RRPFreeVector(temp->values);
		free(temp);

		temp = next;
	}
	p->count = 0;
	p->head = NULL;
	p->tail = NULL;
	p->current = NULL;

	return 0;

} /* RRPClearProperties */






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

int
RRPGetPropertiesSize (
	RRPPROPERTIES* p
) {
	/*
	** Validate parameters
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return - 1;
	}
	return p->count;

} /* RRPGetPropertiesSize */







/*
** FOR INTERNAL USE
**
** Searches for a property based on its key. Returns a pointer
** to the actual node if it exists. NULL is returned if property
** does not exist
**
*/
RRPPROPERTY_NODE*
RRPFindProperty (
	RRPPROPERTIES* p,
	char* key
) {
	RRPPROPERTY_NODE* temp;

	temp = p->head;
	
	while (temp) {
		if (0 == strcmp(temp->key, key)) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
} /* RRPFindProperty */





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

RRPVECTOR*
RRPGetProperty (
	RRPPROPERTIES* p,
	char* key
) {
	RRPPROPERTY_NODE* temp;

	/*
	** Validate parameters
	*/
	if (p == NULL || key == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	if (NULL == (temp = RRPFindProperty(p, key))) {
		return NULL;
	}

	return temp->values;

} /* RRPGetProperty */





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

int
RRPRemoveProperty (
	RRPPROPERTIES* p,
	char* key
) {
	RRPPROPERTY_NODE* temp;
	RRPPROPERTY_NODE* prev;

	/*
	** Validate parameters
	*/
	if (p == NULL || key == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	temp = p->head;
	prev = NULL;
	
	while (temp) {
		if (0 == strcmp(temp->key, key)) {
			if (prev == NULL) {
				p->head = temp->next;
				if (p->current == temp) {
					p->current = temp->next;
				}
			} else {
				prev->next = temp->next;
				if (temp == p->tail) {
					p->tail = prev;
				}
				if (p->current == temp) {
					p->current = prev;
				}
			}
			p->count--;
			free(temp->key);
			RRPFreeVector(temp->values);
			free(temp);

			return 0;
		}
		prev = temp;
		temp = temp->next;
	}

	RRPSetInternalErrorCode(RRP_NO_SUCH_PROPERTY_ERROR);
	return -1;

} /* RRPRemoveProperty */






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

int
RRPResetPropertyPointer (
	RRPPROPERTIES* p
) {
	/*
	** Validate parameters
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	p->current = p->head;
	return 0;

} /* RRPResetPropertyPointer */






/*
**
** Function: RRPGetNextPropertyKey
**
** Description: Returns the key which identifies the current property
**              and moves the current property pointer to the next property in
**              the list. If the current property pointer reaches the end of
**              of the list, then NULL will be returned. The current
**              property pointer can be reset (set to point to the first
**              property in the list) by calling RRPResetPropertyPointer()
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

char*
RRPGetNextPropertyKey (
	RRPPROPERTIES* p
) {
	RRPPROPERTY_NODE* temp;

	/*
	** Validate parameters
	*/
	if (p == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}


	if (p->current == NULL) {
		return NULL;
	}
	temp = p->current;
	p->current = p->current->next;

	return temp->key;

} /* RRPGetNextPropertyKey */






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

int
RRPPutPropertyPair (
	RRPPROPERTIES* p,
	char* data
) {
	char* key = NULL;
	char* value = NULL;
	char* copy = NULL;
	char* index = NULL;
	int result = -1;

	/*
	** Validate parameters
	*/
   if (p == NULL || data == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
      return -1;
   }
   copy = strdup(data);

	if (copy == NULL) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return -1;
	}

	key = copy;

	index = strchr(copy, ':');

	if (index != NULL) {
		*index = '\0';
		value = index + 1;
		result = RRPPutProperty(p, key, value);
	}
	free(copy);

	return result;

} /* RRPPutPropertyPair */






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

RRPBOOLEAN
RRPContainsProperty (
	RRPPROPERTIES* p,
	char* key
) {
	/*
	** Validate parameters
	*/
   if (p == NULL || key == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
      return RRPFALSE;
   }

	if (RRPGetProperty(p, key) != NULL) {
		return RRPTRUE;
	}

	return RRPFALSE;

} /* RRPContainsProperty */



