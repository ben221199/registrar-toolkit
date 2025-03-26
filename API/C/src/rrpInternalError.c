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
** Module Name: rrpInternalError.c
**
** Programmer: Stephen Mahlstedt
** Version: 1.0 for RRP version 1.0
**
** Date: 01/13/1999
**
** Description: This library provides a way to store a predefined internal
**              error code. After an internal error code has been set, the
**              code or the code text description can be retrieved. These
**              error codes and descriptions only represent error internal
**              to the API and do not represent RRP responses (See RRP
**              Documentation).
**
** Entry Points:
**
**    RRPSetInternalErrorCode(RRPINTERNAL_ERROR_CODE)
**    RRPGetInternalErrorCode(void)
**    RRPGetInternalErrorDescription(void)
**    RRPPrintInternalErrorDescription(void)
**
** Changes:
**
*/




#include <stdlib.h>
#include "rrpInternalError.h"


/*
** Static internal error code
*/
static RRPINTERNAL_ERROR_CODE  _errorCode = RRP_NO_ERROR;



/*
** Constant array containing descriptions of each error code
*/
char* rrpErrorDescriptions[] = {
	"No error has been set",
   "Input/output error",
   "Invalid function parameter value",
	"Error allocating memory",
	"No such property in RRPProperties",
	"Vector index is out of bounds",
	"Cannot resolve host name to IP address",
	"Cannot establish connection to host",
	"No socket connection exists",
	"Invalid RRP response format",
	"Unknown internal error",
	"Socket operation timeout"
};




/*
**
** Function: RRPSetInternalErrorCode
**
** Description: Sets the internal error code
**
** Input: RRPINTERNAL_ERROR_CODE - the internal error code to set
**
** Output: none
**
** Return: none
**
**
*/

void
RRPSetInternalErrorCode (RRPINTERNAL_ERROR_CODE code) {
	_errorCode = code;
} /* RRPSetInternalErrorCode */




/*
**
** Function: RRPGetInternalErrorCode
**
** Description: Returns the internal error code
**
** Input: none
**
** Output: none
**
** Return: RRPINTERNAL_ERROR_CODE - the error code. RRP_NO_ERROR is returned
**                                  if an error code has not yet been set
**
**
*/

RRPINTERNAL_ERROR_CODE
RRPGetInternalErrorCode (void) {
	return _errorCode;
} /* RRPInternalGetErrorCode */






/*
**
** Function: RRPGetInternalErrorDescription
**
** Description: Returns a text description of current internal error code
**
** Input: none
**
** Output: none
**
** Return: char* - if an error code has been set the text description of
**                 the error code is returned. If the error code has not
**                 yet been set then NULL is returned
**
**
*/

char*
RRPGetInternalErrorDescription () {
	/*
	** Only attempt to get description from array is _errorCode is
	** a valid array index
	*/
	if (_errorCode < sizeof rrpErrorDescriptions) {
		return rrpErrorDescriptions[_errorCode];
	}
	return NULL;

} /* RRPGetInternalErrorDescription */






/*
**
** Function: RRPPrintInternalErrorDescription
**
** Description: Prints internal error description to standard output. If no
**              error code has been set then nothing will be printed
**
** Input: none
**
** Output: none
**
** Return: none
**
**
*/

void
RRPPrintInternalErrorDescription () {
	char* desc = RRPGetInternalErrorDescription();

	/*
	** Only print something if description is not null
	*/
	if (desc != NULL) {
		printf("%s\n", desc);
	}

} /* RRPPrintInternalErrorDescription */



