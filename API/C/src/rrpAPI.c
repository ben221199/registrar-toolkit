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
** Module Name: rrpAPI.c
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
**       RRPVECTOR*, RRPPROPERTIES*, RRPVECTOR*);
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
** Date:	Oct. 25th, 1999
** Changes:
**
** Provide new arguments registrationPeriod/renewRegistrationPeriod for
** RRP add/renew Domain API. The signature of RRP add/renew commands are
** changed to:
**
** RRPAddDomain( char* domainName, RRPVECTOR* nameServers,...)
** RRPRenewDomain( char* domainName, ...)
**
** To format the new RRP request string, a new function appendIntToRequest
** is added. The signature of the function is:
**
** appendIntToRequest (char*, char*, int)
**
** ========================================================================
**
** Modified by: Lihui Zhang
** Version: 1.1 for RRP version 1.1
**
** Date:	Nov. 22nd, 1999
** Changes:
**
** Add another 'optional option' currentExpirationYear to Renew Domain API.
** Since the new argument is an optional option, it should be in the variable 
** argument list of the function. There is no change of the signature of 
** renew interface.
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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "rrpAPI.h"
#include "rrpInternalError.h"
#include "rrpConnection.h"
#include "rrpProperties.h"
#include "rrpVector.h"

RRPRESPONSE* createResponse (void);
RRPRESPONSE* parseResponse(char*);
RRPRESPONSE* processRequest (char*);
char* appendStringToRequest (char*, char*);
char* appendIntToRequest (char*, char*, int);
char* mySprintf(char*, char*, ...);
char* appendPropertiesToRequest (char*, char*, RRPPROPERTIES*);
char* appendVectorToRequest (char*, char*, RRPVECTOR*);
char* appendDeletedVectorToRequest (char*, char*, RRPVECTOR*);

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
RRPRESPONSE* RRPStartSession (
	char* registrarID,
	char* registrarPassword,
	char* newRegistrarPassword
) {
	char* request = NULL;
	int status = 0;

	/*
	** Validate parameters
	*/
	if (registrarID == NULL || registrarPassword == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	if (newRegistrarPassword != NULL && strlen(newRegistrarPassword) > 0) {
		request = mySprintf("Session\r\n-Id:%s\r\n-Password:%s\r\n"
			"-NewPassword:%s\r\n.\r\n", 
			request, registrarID, registrarPassword, 
			newRegistrarPassword);
	}
	else {
		request = mySprintf("Session\r\n-Id:%s\r\n-Password:%s\r\n.\r\n",
			request, registrarID, registrarPassword);
	}

	if (request == NULL) {
		return NULL;
	}

	printf("%s\r\n", request);

	return processRequest(request);

} /* RRPStartSession */

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
RRPRESPONSE* RRPEndSession () {
   char* request = NULL;

	if ( (request = mySprintf("Quit\r\n.\r\n", request)) == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPEndSession */

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
RRPRESPONSE* RRPAddDomain (
	char* domainName,
	RRPVECTOR* nameServers,
	int registrationPeriod
) {
   char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Add\r\nEntityName:Domain\r\nDomainName:%s\r\n",
		request, domainName);

	if (request == NULL) {
		return NULL;
	}

	if (nameServers != NULL) {
		request = appendVectorToRequest(request, "NameServer", nameServers);
		if (request == NULL) {
			return NULL;
		}
	}

	if ( registrationPeriod > 0) {
		request = appendIntToRequest(request, "-Period:", registrationPeriod);
	   if (request == NULL) {
		 return NULL;
      }
	}

	request = mySprintf(".\r\n", request);
	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPAddDomain */

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
RRPRESPONSE* RRPAddNameServer (
	char* nameServer,
	RRPVECTOR* ipAddresses
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (nameServer == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Add\r\nEntityName:NameServer\r\nNameServer:%s\r\n",
		request, nameServer);
	if (request == NULL) {
		return NULL;
	}

	if (ipAddresses != NULL) {
		request = appendVectorToRequest(request, "IPAddress", ipAddresses);
		if (request == NULL) {
			return NULL;
		}
	}

	request = mySprintf(".\r\n", request);
	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPAddNameServer */

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
RRPRESPONSE* RRPCheckDomain (
	char* domainName
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Check\r\nEntityName:Domain\r\n"
		"DomainName:%s\r\n.\r\n", request, domainName);

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPCheckDomain */

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
RRPRESPONSE* RRPCheckNameServer (
	char* nameServer
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (nameServer == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Check\r\nEntityName:NameServer\r\n"
		"NameServer:%s\r\n.\r\n", request, nameServer);

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPCheckNameServer */

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

RRPRESPONSE* RRPDeleteDomain (
	char* domainName
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Del\r\nEntityName:Domain\r\nDomainName:%s\r\n.\r\n",
		request, domainName);

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPDeleteDomain */

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
RRPRESPONSE* RRPDeleteNameServer (
	char* nameServer
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (nameServer == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Del\r\nEntityName:NameServer\r\n"
		"NameServer:%s\r\n.\r\n", request, nameServer);

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPDeleteNameServer */

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
RRPRESPONSE* RRPDescribe (
	char* target
) {
	char* request = NULL;

	if (target == NULL) {
		request = mySprintf("Describe\r\n.\r\n", request);
	}
	else {
		request = mySprintf("Describe\r\n-Target:%s\r\n.\r\n", 
			request, target);
	}

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPDescribe */

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
RRPRESPONSE* RRPModifyDomain (
	char* domainName,
	RRPVECTOR* addedNameServers,
	RRPPROPERTIES* modifiedNameServers,
	RRPVECTOR* deletedNameServers,
	RRPVECTOR* addedStatuses,
	RRPPROPERTIES* modifiedStatuses,
	RRPVECTOR* deletedStatuses
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}


	request = mySprintf("Mod\r\nEntityName:Domain\r\nDomainName:%s\r\n",
		request, domainName);

	if (request == NULL) {
		return NULL;
	}


	if (addedNameServers != NULL) {
		request = appendVectorToRequest(request, "NameServer", 
			addedNameServers);
		if (request == NULL) {
			return NULL;
		}
	}

	if (modifiedNameServers != NULL) {
		request = appendPropertiesToRequest(request, "NameServer",
			modifiedNameServers);
		if (request == NULL) {
			return NULL;
		}
	}

	if (deletedNameServers != NULL) {
		request = appendDeletedVectorToRequest(request, "NameServer",
			deletedNameServers);
		if (request == NULL) {
			return NULL;
		}
	}

	if (addedStatuses != NULL) {
		request = appendVectorToRequest(request, "Status", addedStatuses);
		if (request == NULL) {
			return NULL;
		}
	}


	if (modifiedStatuses != NULL) {
		request = appendPropertiesToRequest(request, "Status",
			modifiedStatuses);
		if (request == NULL) {
			return NULL;
		}
	}

	if (deletedStatuses != NULL) {
		request = appendDeletedVectorToRequest(request, "Status",
			deletedStatuses);
		if (request == NULL) {
			return NULL;
		}
	}

	request = mySprintf(".\r\n", request);
	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPModifyDomain */

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
RRPRESPONSE* RRPModifyNameServer (
	char* nameServer,
	char* newNameServer,
	RRPVECTOR* addedIPAddresses,
	RRPPROPERTIES* modifiedIPAddresses,
	RRPVECTOR* deletedIPAddresses
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (nameServer == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Mod\r\nEntityName:NameServer\r\nNameServer:%s\r\n",
		request, nameServer);

	if (request == NULL) {
		return NULL;
	}


	if (newNameServer != NULL) {
		request = mySprintf("NewNameServer:%s\r\n", request, newNameServer);
		if (request == NULL) {
			return NULL;
		}
	}

	if (addedIPAddresses != NULL) {
		request = appendVectorToRequest(request, "IPAddress", addedIPAddresses);
		if (request == NULL) {
			return NULL;
		}
	}

	if (modifiedIPAddresses != NULL) {
		request = appendPropertiesToRequest(request, "IPAddress",
			modifiedIPAddresses);
		if (request == NULL) {
			return NULL;
		}
	}

	if (deletedIPAddresses != NULL) {
		request = appendDeletedVectorToRequest(request, "IPAddress",
			deletedIPAddresses);
		if (request == NULL) {
			return NULL;
		}
	}

	request = mySprintf(".\r\n", request);
	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPModifyNameServer */

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
RRPRESPONSE* RRPRenewDomain (
	char* domainName,
	int renewRegistrationPeriod,
	int currentExpirationYear
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Renew\r\nEntityName:Domain\r\nDomainName:%s\r\n",
		request, domainName);

	if (request == NULL) {
		return NULL;
	}

	if (renewRegistrationPeriod > 0) {
		request = appendIntToRequest(request, "-Period:", 
			renewRegistrationPeriod);
		if (request == NULL) {
			return NULL;
		}
	}

	if (currentExpirationYear > 0) {
		request = appendIntToRequest(request, "-CurrentExpirationYear:", 
			currentExpirationYear);
		if (request == NULL) {
			return NULL;
		}
	}

	request = mySprintf(".\r\n", request);
	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPRenewDomain */

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
*/
RRPRESPONSE* RRPStatusDomain (
	char* domainName
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Status\r\nEntityName:Domain\r\nDomainName:%s\r\n.\r\n",
		request, domainName);

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPStatusDomain */

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
*/
RRPRESPONSE* RRPStatusNameServer (
	char* nameServer
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (nameServer == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Status\r\nEntityName:NameServer\r\nNameServer:%s",
		request, nameServer);

	request = mySprintf("\r\n.\r\n", request);

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPStatusNameServer */

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
*/
RRPRESPONSE* RRPTransferDomain (
	char* domainName,
	char* approve
) {
	char* request = NULL;

	/*
	** Validate parameters
	*/
	if (domainName == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	request = mySprintf("Transfer\r\nEntityName:Domain\r\nDomainName:%s\r\n",
		request, domainName);

	if (request == NULL) {
		return NULL;
	}

	if (approve != NULL) {
		request = mySprintf("-Approve:%s\r\n.\r\n", request, approve);
	}
	else {
		request = mySprintf(".\r\n", request);
	}

	if (request == NULL) {
		return NULL;
	}

	return processRequest(request);

} /* RRPTransferDomain */

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
int RRPFreeResponse (
	RRPRESPONSE* response
) {
	/*
	** Validate parameter
	*/
	if (response == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	if (response->description != NULL) {
		free(response->description);
	}

	if (response->attributes != NULL) {
		RRPFreeProperties(response->attributes);
	}

	free(response);

	return 0;

} /* RRPFreeResponse */

/*
** Parses an RRP response string and builds an RRPRESPONSE structure.
** Returns a pointer to new RRPRESPONSE structure. Returns NULL and sets error
** code if an error occurs.
**
** Note:        IMPORTANT: THE MEMORY THAT IS ALLOCATED FOR EACH RRPRESPONSE
**              STRUCTURE THAT IS CONSTRUCTED BY EACH FUNCTION MUST BE
**              RELEASED BY CALLING THE RRPFreeResponse() FUNCTION
**              (SEE FUNCTION DESCRIPTION BELOW)
*/
RRPRESPONSE* parseResponse (
   char* string 
) {
	RRPRESPONSE* response = NULL;
	char* responseString = NULL;
	char* crlfIndex = NULL;
	char* spaceIndex = NULL;
	char* line = NULL;
	char c;

	/*
	** Validate parameter
	*/
	if (string == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return NULL;
	}

	/*
	** make a copy of 'string' parameter so we can manipulate it without
	** altering the parameter
	*/
	responseString = strdup(string);

	/*
	** Start line pointer at beginning of string
	*/
	line = responseString;

	/*
	** Allocate memory for RRPRESONSE structure... check for
	** memory allocation error
	*/
	response = createResponse();

	if (NULL == response) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return NULL;
	}

	while ((crlfIndex = strstr(line, "\r\n")) != NULL) {
		c = *crlfIndex;
		*crlfIndex = '\0';

		if (response->code == -1) {
			spaceIndex = strstr(line, " ");
			if (spaceIndex	== NULL) {
				RRPSetInternalErrorCode(RRP_RESPONSE_FORMAT_ERROR);
				break;
			}

			*spaceIndex = '\0';
			response->code = atoi(line);
			response->description = strdup(spaceIndex + 1);

			if (response->description == NULL) {
				RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
				break;
			}
		}
		else if (strchr(line, ':') != NULL) {
			if (response->attributes == NULL) {
				response->attributes = RRPCreateProperties();
			}
			RRPPutPropertyPair(response->attributes, line);
		}
		else if (strcmp(line, ".\r\n")) {
   			free(responseString);
	   		return response;
		}
		else {
			RRPSetInternalErrorCode(RRP_RESPONSE_FORMAT_ERROR);
			break;
		}

		line = crlfIndex + 2;
		*crlfIndex = c;
	}

	/*
	** If we made it this far, then an error occured. Free up any allocated
	** resources and return nULL
	*/
	if (response != NULL) {
		free(response);
	}

	if (responseString != NULL) {
		free(responseString);
	}
	
	return NULL;
}

RRPRESPONSE* createResponse () {
	RRPRESPONSE* response = NULL;
	response = (RRPRESPONSE*) calloc(1, sizeof(RRPRESPONSE));

	if (response == NULL) {
		return NULL;
	}

	response->code = -1;
	response->description = NULL;
	response->attributes = NULL;

	return response;

} /* createResponse */


char*
appendIntToRequest (
	char* orginalRequest,
	char* key,
	int   newInteger
) {
   char* newRequest = NULL;
	char newString[MAX_SIZE];
   int  originalRequestSize, newStringSize;
	
	sprintf( newString, "%s%d\r\n", key, newInteger );
	newStringSize = strlen(newString);

   if (orginalRequest == NULL) {
      originalRequestSize = 0;
      newRequest = (char*) calloc(1, newStringSize + originalRequestSize + 1);
   }
   else {
      originalRequestSize = strlen(orginalRequest);
      newRequest = (char*) realloc(orginalRequest, newStringSize + originalRequestSize + 1);
   }

   if (newRequest == NULL) {
      RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
      return NULL;
   }
   memcpy(newRequest + originalRequestSize, newString, newStringSize + 1);
   return newRequest;
}






char*
appendStringToRequest (
	char* orginalRequest,
	char* newString
) {
	char* newRequest = NULL;
	size_t originalRequestSize;
	size_t newStringSize;

	if (NULL == newString) {
		return NULL;
	}
	else {
		newStringSize = strlen(newString);
	}

	if (orginalRequest == NULL) {
		originalRequestSize = 0;
		newRequest = (char*) calloc(1, newStringSize + originalRequestSize + 1);
	}
	else {
		originalRequestSize = strlen(orginalRequest);
		newRequest = (char*) realloc(orginalRequest, newStringSize +
			originalRequestSize + 1);
	}

	if (newRequest == NULL) {
		RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
		return NULL;
	}
	memcpy(newRequest + originalRequestSize, newString, newStringSize + 1);
	return newRequest;

} /* appendStringToRequest */




char*
mySprintf (
	char* format,
	char* originalString,
	...
) {
	va_list argumentList;
	char* stringArgument;
	RRPPROPERTIES* propertiesArgument;
	char* newString = NULL;
	char modifier[3];
	char string[64];
	int stringIndex = 0;

	modifier[0] = '\0';
	modifier[1] = '\0';
	modifier[2] = '\0';
	string[0] = '\0';


	va_start(argumentList, originalString);

	newString = originalString;

	while (*format != '\0') {
		modifier[0] = modifier[1];
		modifier[1] = *format;

		if (strcmp(modifier, "%s") == 0) {
			stringArgument = va_arg(argumentList, char*);
			if (string[0] != '\0') {
				newString = appendStringToRequest(newString, string);
				if (newString == NULL) {
					return NULL;
				}
			}
			stringIndex = 0;
			string[0] = '\0';

			newString = appendStringToRequest(newString, stringArgument);
			if (newString == NULL) {
				return NULL;
			}
		}
		else if (*format != '%') {
			string[stringIndex++] = *format;
			string[stringIndex] = '\0';
		}

		format++;
	}


	if (string[0] != '\0') {
		newString = appendStringToRequest(newString, string);
		if (newString == NULL) {
			return NULL;
		}
	}

	va_end(argumentList); 
	return newString;

} /* mySprintf */







char*
appendPropertiesToRequest (
	char* originalRequest,
	char* rrpAttributeKey,
	RRPPROPERTIES* properties
) {
	RRPPROPERTIES* clone = NULL;
	RRPVECTOR* vector = NULL;
	char* newRequest = NULL;
	int vectorSize = 0;
	char* value = NULL;
	char* key = NULL;
	int i = 0;

	if ( (clone = RRPCloneProperties(properties)) == NULL) {
		return NULL;
	}
	if (RRPResetPropertyPointer(clone) == -1) {
		return NULL;
	}

	newRequest = originalRequest;

	while ( (key = RRPGetNextPropertyKey(clone)) != NULL) {
		vector = RRPGetProperty(clone, key);
		if (vector == NULL) {
			return NULL;
		}

		vectorSize = RRPGetVectorSize(vector);

		for (i=0; i < vectorSize; i++) {
			value = RRPGetVectorElementAt(vector, i);
			if (value == NULL) {
				return NULL;
			}
			newRequest = mySprintf("%s:%s=%s\r\n", newRequest, rrpAttributeKey,
				key, value);
			if (newRequest == NULL) {
				RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
				return NULL;
			}
		}
	}

	RRPFreeProperties(clone);

	return newRequest;

} /* appendPropertiesToRequest */







char*
appendVectorToRequest (
	char* originalRequest,
	char* key,
	RRPVECTOR* vector
) {
	int vectorSize = 0;
	char* newRequest = NULL;
	char* value = NULL;
	int i = 0;

	vectorSize = RRPGetVectorSize(vector);

	newRequest = originalRequest;

	for (i=0; i < vectorSize; i++) {
		value = RRPGetVectorElementAt(vector, i);
		if (value == NULL) {
			return NULL;
		}
		newRequest = mySprintf("%s:%s\r\n", newRequest, key, value);
		if (newRequest == NULL) {
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}
	}

	return newRequest;

} /* appendVectorToRequest */



char*
appendDeletedVectorToRequest (
	char* originalRequest,
	char* key,
	RRPVECTOR* vector
) {
	int vectorSize = 0;
	char* newRequest = NULL;
	char* value = NULL;
	int i = 0;

	vectorSize = RRPGetVectorSize(vector);

	newRequest = originalRequest;

	for (i=0; i < vectorSize; i++) {
		value = RRPGetVectorElementAt(vector, i);
		if (value == NULL) {
			return NULL;
		}
		newRequest = mySprintf("%s:%s=\r\n", newRequest, key, value);
		if (newRequest == NULL) {
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}
	}

	return newRequest;

} /* appendDeletedVectorToRequest */







RRPRESPONSE*
processRequest (
   char* request
) {
	char* responseString = NULL;
	RRPRESPONSE* response = NULL;

	if (request == NULL) {
		RRPSetInternalErrorCode(RRP_UNKNOWN_ERROR);
		return NULL;
	}

   if (RRPSendRequest(request) < 0) {
		free(request);
		return NULL;
   }

	responseString = RRPReadResponse();


	if (responseString == NULL) {
		free(request);
		return NULL;
	}

	response = parseResponse(responseString);

	free(responseString);
	free(request);

	return response;

} /* processRequest */
