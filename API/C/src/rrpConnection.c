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
** Module Name: rrpConnection.c
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
**              be accessed and interpretted by the functions defined in
**              rrpInternalError.h (see API documentation)
**
** Entry Points:
**
**    RRPCreateConnection (char*, unsigned short int);
**    RRPSendRequest(char*);
**    RRPReadResponse();
**    RRPCloseConnection();
**
** IMPORTANT: THIS IMPLEMENTATION OF THE FUNCTIONS CONTAINED
** WITHIN THIS FILE ARE NOT SSL-ENABLED. THE FUNCTION SIGNATURES MUST
** BE MAINTAINED, BUT THE IMPLEMENTATIONS MUST BE REPLACED WITH SSL-ENABLED
** CODE. WHEN THE API IS BUILT USING THE PROVIDED MAKEFILE, TWO SHARED
** OBJECT FILES ARE CREATED: librrpapi.so AND librrpconnection.so
** THE FORMER CAN REMAIN AS IS WHILE THE LATTER SHOULD BE REPLACED
** WITH A SECURE VERSION
**
** Changes:
**
*/


#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include "rrpConnection.h"
#include "rrpInternalError.h"


static int _socket = -1;
static unsigned _timeout = 0;
static int _timeoutFlag = 0;
void disableTimeoutAlarm (void);
void sigAlarmHandler();



/*
** Internal function declaration
**
** Copies a host name or ip address string into an in_addr structure
*/
int GetInAddrFromString ( struct in_addr*, char*);


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
void RRPSetTimeout (
	unsigned timeout
) {
	/*
	** store timeout setting so the RRPSendRequest and RRPReadResponse
	** funtions will have access to it
	*/
	_timeout = timeout;
}




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
**               internal error or timeout occurs.
**
**
*/

int
RRPCreateConnection (
	char* host,
	unsigned short int port
) {
	struct in_addr addr;
	struct sockaddr_in address;
	char* welcomeMessage = NULL;


	/*
	** Validate parameters
	*/
	if (host == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	/*
	** Populate in_addr structure
	*/
	if (GetInAddrFromString(&addr, host) < 0) {
		RRPSetInternalErrorCode(RRP_INVALID_HOST_NAME_ERROR);
		return -1;
	}


	/*
	** set address
	*/
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = addr.s_addr;

	_socket = socket(AF_INET, SOCK_STREAM, 0);

	/*
	** if a timeout option is set, set timeout alarm before entering
	** blocked write
	*/
	if (_timeout > 0) {
		_timeoutFlag = 0;
		signal(SIGALRM, sigAlarmHandler);
		alarm(_timeout);
	}

	if (connect(_socket, (struct sockaddr *) &address,sizeof(address)) < 0) {
		if (_timeoutFlag == 1) {
			RRPSetInternalErrorCode(RRP_TIMEOUT_ERROR);
		}
		else {
			disableTimeoutAlarm();
			RRPSetInternalErrorCode(RRP_SOCKET_CONNECT_ERROR);
		}
		return -1;
	}

	disableTimeoutAlarm();

	/*
	** Read welcome message from RRP server
	*/
	welcomeMessage = RRPReadResponse();
	if (welcomeMessage == NULL) {
		return -1;
	}
	free(welcomeMessage);
	

	return 0;

}





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
**               internal error or timeout occurs.
**
**
*/

int
RRPSendRequest (
	char* request
) {
	size_t requestSize;

	/*
	** Validate parameters
	*/
	if (request == NULL) {
		RRPSetInternalErrorCode(RRP_BAD_PARAM_ERROR);
		return -1;
	}

	/*
	** Make sure that we're actually connected to a server before
	** attempting to write data to socket
	*/
	if (_socket < 0) {
		RRPSetInternalErrorCode(RRP_NOT_CONNECTED_ERROR);
		return -1;
	}

	/*
	** Determine size of request string
	*/
	requestSize = strlen(request);

	/*
	** if a timeout option is set, set timeout alarm before entering
	** blocked write
	*/
	if (_timeout > 0) {
		_timeoutFlag = 0;
		signal(SIGALRM, sigAlarmHandler);
		alarm(_timeout);
	}


	/*
	** Write request string to socket and make sure that all bytes
	** were written
	*/
	if (write(_socket, request, requestSize) < (ssize_t) requestSize) {
		if (_timeoutFlag == 1) {
			RRPSetInternalErrorCode(RRP_TIMEOUT_ERROR);
		}
		else {
			disableTimeoutAlarm();
			RRPSetInternalErrorCode(RRP_IO_ERROR);
		}
		return -1;
	}

	disableTimeoutAlarm();
	return 0;
}







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
**                 NULL is returned if an internal error or timeout occurs.
**
** Note: THE MEMORY USED FOR THE RRP RESPONSE STRING IS ALLOCATED DYNAMICALLY
**       AND MUST BE RELEASED BY THE CALLER OF THE FUNCTION
**
*/

char*
RRPReadResponse () {
	char inputBuffer[RRPBUFSIZE + 1];
	int byteCount;
	char* outputBuffer = NULL;
	char* end = NULL;
	size_t outputSize = NULL;

	/*
	** if a timeout option is set, set timeout alarm before entering
	** blocked read
	*/
	if (_timeout > 0) {
		_timeoutFlag = 0;
		signal(SIGALRM, sigAlarmHandler);
		alarm(_timeout);
	}


	while ((byteCount = read(_socket, &inputBuffer, RRPBUFSIZE)) > 0) {
		inputBuffer[byteCount] = '\0';

		if (outputBuffer == NULL) {
			outputBuffer = (char*) calloc(1, byteCount + 1);
		}
		else {
			outputSize = strlen(outputBuffer);
			outputBuffer = (char*) realloc(outputBuffer,
				outputSize + byteCount + 1);
		}

		if (outputBuffer == NULL) {
			disableTimeoutAlarm();
			RRPSetInternalErrorCode(RRP_MEM_ALLOC_ERROR);
			return NULL;
		}

		memcpy(outputBuffer + outputSize, inputBuffer, byteCount + 1);

		end = strstr(outputBuffer, "\r\n.\r\n");


		if (end != NULL) {
			disableTimeoutAlarm();
			*(end + 5) = '\0';
			break;
		}

		/*
		** Reset alarm before entering another blocked read
		*/
		if (_timeout > 0) {
			_timeoutFlag = 0;
			signal(SIGALRM, sigAlarmHandler);
			alarm(_timeout);
		}
	}


	/*
	** Check to make sure that loop did not exit because of an IO error
	*/
	if (byteCount < 0 || outputBuffer == NULL) {
		/*
		** Check to see if the error occured due to a timeout
		*/
		if (_timeoutFlag == 1) {
			RRPSetInternalErrorCode(RRP_TIMEOUT_ERROR);
		}
		else {
			disableTimeoutAlarm();
			RRPSetInternalErrorCode(RRP_IO_ERROR);
		}
		return NULL;
	}

	disableTimeoutAlarm();
	return outputBuffer;

} /* RRPReadResponse */






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

int
RRPCloseConnection () {
	int result = -1;

	/*
	** Make sure that we're actually connected to a server before
	** attempting to close socket
	*/
	if (_socket < 0) {
		RRPSetInternalErrorCode(RRP_NOT_CONNECTED_ERROR);
		return -1;
	}

	shutdown(_socket, 2);
	result = close(_socket);
	_socket = -1;

	return result;
}






/*
** For internal use only
*/
int
GetInAddrFromString (
	struct in_addr* saddr,
	char* address
) {
	struct hostent* host;

	/*
	**
	** First try it as n.n.n.n
	** where n = [0 -255]
	**
	*/
	saddr->s_addr = inet_addr(address);
	if (saddr->s_addr != (in_addr_t)-1) {
		return 0;
	}

	host = gethostbyname(address);

	if (host != NULL) {
		memcpy(saddr, *host->h_addr_list, sizeof(struct in_addr));
		return 0;
	}

	return -1;

} /* GetInAddrFromString */






/*
** For internal use only
*/
void disableTimeoutAlarm () {
	_timeoutFlag = 0;
	alarm(0);
} /* disableTimeoutAlarm */




/*
** For internal use only
*/
void
sigAlarmHandler () {
	_timeoutFlag = 1;

} /* sigAlarmHandler */

