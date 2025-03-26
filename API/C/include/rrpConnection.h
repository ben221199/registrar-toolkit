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
** Module Name: rrpConnection.h
**
** Programmer: Stephen Mahlstedt
** Version: 1.0 for RRP version 1.0
**
** Date: 01/13/1999
**
** Description: rrpConnection provides an API for establishing a connection
**              to an RRP server, as well as sending RRP requests and reading
**              RRP responses from that server.
**
**              In the event of an internal error (IO error, memory
**              allocation error, etc.) each function will return a
**              value indicating that an error has occured (see function
**              descriptions below). An internal error code that
**              identifies the error will be set. The error code can
**              be accessed and interpreted by the functions defined in
**              rrpInternalError.h (see API documentation)
**
** Entry Points:
**
**  RRPSetTimeout (unsigned);
** 	RRPCreateConnection (char*, unsigned short int);
** 	RRPSendRequest(char*);
** 	RRPReadResponse();
** 	RRPCloseConnection();
**
**
**  IMPORTANT: THIS IMPLEMENTATION OF THE RRP API IS NOT
**  SSL ENABLED BY DEFAULT. THE FUNCTION PROTOTYPES LISTED
**  WITHIN THIS HEADER FILE MUST BE IMPLEMENTED WITHIN THE
**  rrpConnection.c FILE. THE CURRENT FUNCTION IMPLEMENTATIONS
**  FOUND IN rrpConnection.c MUST BE REPLACED WITH SSL_ENABLED CODE.
**  WHEN THE API IS BUILT USING THE PROVIDED MAKEFILE, TWO SHARED
**  OBJECT FILES ARE CREATED: librrpapi.so AND librrpconnection.so.
**  THE FORMER CAN REMAIN AS IS WHILE THE LATTER SHOULD BE REPLACED
**  WITH A SECURE VERSION
**
** Changes:
**
*/

#ifndef _RRP_CONNECTION_H_
#define _RRP_CONNECTION_H_

#ifndef RRPBUFSIZE
	#define RRPBUFSIZE 256
#endif

/*
** Function: RRPSetTimeout
**
** Description: sets a timeout value (in seconds) for socket operations.
**
** Input: unsigned - timeout value (in seconds) for socket operations. A
**                    value of 0 disables timeout option
**
** Output: none
**
** Return: void
**
*/
void RRPSetTimeout (unsigned timeout);

/*
**
** Function: RRPCreateConnection
**
** Description: Establishes a connection to a specified RRP server
**
** Input: char* - host name or IP address of RRP server
**        unsigned short int - RRP server port
**
** Output: none
**
** Return: int - 0 is returned if successful. -1 is returned if an
**               internal error occurs.
**
**
*/
int RRPCreateConnection (char*, unsigned short int);

/*
**
** Function: RRPSendRequest
**
** Description: Sends a text RRP request string to the RRP server
**
** Input: char* - the RRP request string to send to the server
**
** Output: none
**
** Return: int - 0 is returned if successful. -1 is returned if an
**               internal error occurs.
**
**
*/
int RRPSendRequest (char*);

/*
**
** Function: RRPReadResponse
**
** Description: reads and returns a RRP response string from the server
**
** Input: none
**
** Output: none
**
** Return: char* - the RRP response string is returned if successful.
**                 NULL is returned if an internal error occurs.
**
** Note: THE MEMORY USED FOR THE RRP RESPONSE STRING IS ALLOCATED DYNAMICALLY
**       AND MUST BE RELEASED BY THE CALLER OF THE FUNCTION
** 
**
*/
char* RRPReadResponse (void);

/*
**
** Function: RRPCloseConnection
**
** Description: Closes connection to RRP server
**
** Input: none
**
** Output: none
**
** Return: int - 0 is returned if successful. -1 is returned if an
**               internal error occurs.
**
**
*/
int RRPCloseConnection (void);

#endif /* _RRP_CONNECTION_H_ */
