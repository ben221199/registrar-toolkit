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
** Module Name: rrpAPI.h
**
** Programmer: Stephen Mahlstedt
** Version: 1.0 for RRP version 1.0
**
** Date: 01/13/1999
**
** Description: rrpAPI provides an API for issuing RRP commands. In order
**              to use any of these functions, a successful call to
**              RRPCreateConnection() must be made (see rrpConnection.h)
**              in order to establish a connection to an RRP server.
**              Each function builds an appropriate RRP command string
**              (see RRP documentation) based on its purspose and its
**              parameters. Once the command string is constructed,
**              it is sent to the RRP server and the RRP text response
**              string is read from the server and is broken up into its
**              components. The components of the RRP response are stored
**              in an RRPRESPONSE structure and a pointer to the structure
**              is returned.
**
**              In the event of an internal error (bad parameter, memory
**              allocation error, etc.) each function will return a
**              value indicating that an error has occured (see function
**              descriptions below). An internal error code that
**              identifies the error will then be set. The error code can
**              be accessed and interpreted by the functions defined in
**              rrpInternalError.h (see API documentation)
**
** Note:        IMPORTANT: THE MEMORY THAT IS ALLOCATED FOR EACH RRPRESPONSE
**              STRUCTURE THAT IS CONSTRUCTED BY EACH FUNCTION MUST BE
**              RELEASED BY CALLING THE RRPFreeResponse() FUNCTION
**              (SEE FUNCTION DESCRIPTION BELOW)
**
**              IMPORTANT: THIS IMPLEMENTATION OF THE RRP API IS NOT
**              SSL ENABLED BY DEFAULT. THE rrpConnection.c file MUST BE
**              REPLACED WITH A SECURE IMPLEMENTATION OF THE FUNCTIONS
**              PROTOTYPED IN THE rrpConnection.h HEADER FILE. WHEN THE
**              API IS BUILT USING THE PROVIDED MAKEFILE, TWO SHARED
**              OBJECT FILES ARE CREATED: librrpapi.so AND librrpconnection.so
**              THE FORMER CAN REMAIN AS IS WHILE THE LATTER SHOULD BE REPLACED
**              WITH A SECURE VERSION
**
**
** Entry Points:
**
**   RRPStartSession(char*, char*, char*);
**   RRPEndSession(void);
**   RRPAddDomain(char*, RRPVECTOR*, int);
**   RRPAddNameServer(char*, RRPVECTOR*);
**   RRPCheckDomain(char*);
**   RRPCheckNameServer(char*);
**   RRPDeleteDomain(char*);
**   RRPDeleteNameServer(char*);
**   RRPDescribe(char*);
**   RRPModifyDomain(char*, RRPVECTOR*, RRPPROPERTIES*, RRPVECTOR*,
**      RRPVECTOR*, RRPPROPERTIES*, RRPVECTOR*);
**   RRPModifyNameServer(char*, char*, RRPVECTOR*, RRPPROPERTIES*, RRPVECTOR*);
**   RRPRenewDomain(char*, int, int);
**   RRPStatusDomain(char*);
**   RRPStatusNameServer(char*);
**   RRPTransferDomain(char*, char*);
**   RRPFreeResponse(RRPRESPONSE*);
**
** ========================================================================
**
** Modified by: Lihui Zhang
** Version: 1.1 for RRP version 1.1
**
** Date: Oct. 25th, 1999
** Changes:
**
** Provide new arguments registrationPeriod/renewRegistrationPeriod for 
** RRP add/renew Domain API. The signature of RRP add/renew commands are 
** changed to:
**
** RRPAddDomain( char* domainName, RRPVECTOR* nameServers,...)
** RRPRenewDomain( char* domainName, ...)
**
** ========================================================================
** 
** Modified by: Chris Bason
** Version: 1.1 for RRP version 1.1
**
** Date: July 17, 2000
** Changes:
** 
** The RRPStartSession, RRPAddDomain, and RRPRenewDomain functions have been
** altered. The variable parameter lists have been removed from these functions
** due to their invalid use of the va_start, va_args, and va_end macros.
** Each function has a new prototype and is listed below:
**
** RRPRESPONSE* RRPStartSession(char*, char*, char *);
** RRPRESPONSE* RRPAddDomain(char*, RRPVECTOR*, int);
** RRPRESPONSE* RRPRenewDomain(char *, int, int);
**
*/

#ifndef _RRP_API_H_
#define _RRP_API_H_

#include "rrpProperties.h"
#include "rrpVector.h"
#include "rrpInternalError.h"
#include "rrpConnection.h"

#ifndef _RRP_BOOLEAN_
#define _RRP_BOOLEAN_

	typedef enum { RRPFALSE, RRPTRUE }  RRPBOOLEAN;

#endif

#ifndef _RRP_RESPONSE_
#define _RRP_RESPONSE_

	typedef struct {
		int code;                  /* RRP response code */
		char* description;         /* RRP response description */
		RRPPROPERTIES* attributes; /* RRP response attributes */
	} RRPRESPONSE;

#endif

#ifndef MAX_SIZE
	#define MAX_SIZE 1024
#endif

/*
**
** Function: RRPStartSession
**
** Description: Creates an autheticated RRP session.
**
** Input: char* - registrar's id
**        char* - regsitrar's password
**        char* - optional new password for registrar. This parameter
**                must be a valid char * or NULL if the password is not
**                changing.
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPStartSession(char*, char*, char*);

/*
**
** Function: RRPEndSession
**
** Description: Closes an RRP connection
**
** Input: none
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPEndSession(void);

/*
**
** Function: RRPAddDomain
**
** Description: Register a domain name
**
** Input: char* - a fully qualified domain name to register
**        RRPVECTOR* - an optional list of host names of the name servers
**                     that will be hosting this domain. NULL indicates
**                     that there are no name servers hosting this domain
**        int - the registration period of the domain being registered. This 
**              parameter can be set to -1 or 0 if the registration period
**              is to be set to the default registration period by the 
**              RRP Server.
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPAddDomain(char*, RRPVECTOR*, int);

/*
**
** Function: RRPAddNameServer
**
** Description: Register a new name server
**
** Input: char* - a fully qualified host name of a name server to register
**        RRPVECTOR* - a list of one or more IP addresses of the name server.
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPAddNameServer(char*, RRPVECTOR*);

/*
**
** Function: RRPCheckDomain
**
** Description: Check the availability of a domain name
**
** Input: char* - a fully qualified domain name to check
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPCheckDomain(char*);

/*
**
** Function: RRPCheckNameServer
**
** Description: Check the availability of a name server
**
** Input: char* - the full host name of a name server
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPCheckNameServer(char*);

/*
**
** Function: RRPDeleteDomain
**
** Description: Delete the registration of a domain name
**
** Input: char* - a fully qualified domain name to delete
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPDeleteDomain(char*);

/*
**
** Function: RRPDeleteNameServer
**
** Description: Delete the registration of a name server
**
** Input: char* - a fully qualified host name of a name server to delete
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPDeleteNameServer(char*);

/*
**
** Function: RRPDescribe
**
** Description: Get general information about RRP
**
** Input: char* - an optional target indicating the type of information
**                returned (e.g. 'Protocol'). If NULL then the protocol
**                version will be returned in the RRPRESPONSE structure
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPDescribe(char*);

/*
**
** Function: RRPModifyDomain
**
** Description: Update a registered domain name
**
** Input: char* - A fully qualified domain name to update
**        RRPVECTOR* - an optional list of host names of name servers to
**                     add to the domain. NULL indicates that there are no
**                     name servers to be added
**        RRPPROPERTIES* - an optional list containing servers to replace.
**                         Each property 'key' indicates the old server to
**                         replace, and the 'value' indicates the new server
**                         with which to replace it with. NULL indicates
**                         that there are no name servers to be replaced
**        RRPVECTOR* - an optional list of host names of name servers to
**                     delete from the domain. NULL indicates that there are
**                     no name servers to delete
**        RRPVECTOR* - an optional list domain statuses to
**                     add to the domain. NULL indicates that there are no
**                     statuses to be added
**        RRPPROPERTIES* - an optional list containing statuses to replace.
**                         Each property 'key' indicates the old status to
**                         replace, and the 'value' indicates the new status
**                         with which to replace it with. NULL indicates
**                         that there are no statuses to be replaced
**        RRPVECTOR* - an optional list of domain statuses to
**                     delete from the domain. NULL indicates that there are
**                     no statuses to delete
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPModifyDomain(char*, RRPVECTOR*, RRPPROPERTIES*,
	RRPVECTOR*, RRPVECTOR*, RRPPROPERTIES*, RRPVECTOR*);

/*
**
** Function: RRPModifyNameServer
**
** Description: Update the attributes of a registered name server
**
** Input: char* - the fully qualified host name of a name server to update
**        char* - an optional new host name for name server. NULL indicates
**                that the host name of the name server will remain as is
**        RRPVECTOR* - an optional list of IP addresses to add to name
**                     server. NULL indicates that there are no IP addresses
**                     to add
**        RRPPROPERTIES* - an optional list containing IP addresses to replace.
**                         Each property 'key' indicates the old address to
**                         replace, and the 'value' indicates the new address
**                         with which to replace it with. NULL indicates that
**                         there are no IP addresses to replace
**        RRPVECTOR* - an optional list of IP addresses to delete from name
**                     server. NULL indicates that there are no IP addresses
**                     to delete
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPModifyNameServer(char*, char*, RRPVECTOR*, RRPPROPERTIES*,
	RRPVECTOR*);

/*
**
** Function: RRPRenewDomain
**
** Description: Renew the registration of a domain name
**
** Input: char* - the fully qualified domain name to renew
**        int - the renewal registration period of the domain being renewed.
**              This parameter can be set to -1 or 0 if the renewal 
**              registration period is to be set to the default renewal  
**              registration period by the RRP Server.
**        int - the current expiration year of the domain being renewed. This
**              parameter can be set to -1 or 0 if the parameter is not
**              required.
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
*/
RRPRESPONSE* RRPRenewDomain(char*, int, int);

/*
**
** Function: RRPStatusDomain
**
** Description: Query for the attributes of a domain name
**
** Input: char* - the fully qualified domain name to query
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
**
*/
RRPRESPONSE* RRPStatusDomain(char*);

/*
**
** Function: RRPStatusNameServer
**
** Description: Query for the attributes of a name server
**
** Input: char* - the fully qualified host name of name server to query
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
**
*/
RRPRESPONSE* RRPStatusNameServer(char*);

/*
**
** Function: RRPTransferDomain
**
** Description: Has two possible uses: A registrar can use
**              this function to request the transfer of a domain from
**              another registrar to itself. Also, a registrar can use
**              this function to approve or deny any requests that have
**              been previously issued for the transfer of a domain from
**              the registrar to another registrar (see RRP documentation)
**              
**              
**
** Input: char* - if the seconds parameter is NULL, then this parameter
**                specifies a domain name to request the transfer of. If
**                the second parameter is provided then this parameter
**                indicates the domain name whose transfer is being accepted
**                or denied.
**                
**        char* - if requesting the transfer of a domain then this
**                parameter value should be NULL. If approving the request
**                of another registrar to transfer one of your domains, pass
**                "yes". If denying the request of another registrar to
**                transfer one of your domains, pass "no"
**
** Output: none
**
** Return: RRPRESPONSE* - a pointer to an RRPRESPONSE structure containing
**                        the components of the RRP response returned from
**                        the server. NULL is return is an internal error
**                        occurs.
**
** Note: THE MEMORY ALLOCATED FOR THE RRPRESPONSE STRUCTURE MUST BE
**       RELEASED BY CALLING THE RRPFreeResponse() FUNCTION (SEE FUNCTION
**       DESCRIPTION BELOW)
**
**
*/
RRPRESPONSE* RRPTransferDomain(char*, char*);

/*
**
** Function: RRPFreeResponse
**
** Description: Frees all memory allocated for an RRPRESPONSE structure
**
** Input: RRPRESPONSE* - pointer to an RRPRESPONSE structure to free
**
** Output: none
**
** Return: RRPRESPONSE* - returns 0 if successful. Returns -1 is an
**                        internal error occurs
**
*/
int RRPFreeResponse(RRPRESPONSE*);

#endif /* _RRP_API_H_ */
