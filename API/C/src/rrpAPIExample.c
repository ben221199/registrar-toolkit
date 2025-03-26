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
** Test driver for rrpAPI.c
**
*/

#include <stdio.h>
#include <string.h>
#include "rrpAPI.h"

int
main () {

	RRPRESPONSE* response = NULL;
	RRPVECTOR* nameServers = NULL;
	RRPVECTOR* ipAddresses = NULL;

	/*
	** create an RRP connection
	*/
	if (RRPCreateConnection("samplerrpserver", 648) < 0) {
		RRPPrintInternalErrorDescription();
		exit(1);
	}

	/*
	** Establish authenticated session
	*/
	response = RRPStartSession("registrarId", "registrarPassword", NULL);

	if (response == NULL) {
		RRPPrintInternalErrorDescription();
		exit(1);
	}
	else if (response->code != 200) {
		printf("Error creating RRP session: %s\n", response->description);
		exit(1);
	}

	RRPFreeResponse(response);
	printf("Session established successfully\n");

	/*
	** Register new domain name
	*/
	nameServers = RRPCreateVector();
	RRPAddVectorElement(nameServers, "ns3.dn.net");
	RRPAddVectorElement(nameServers, "ns4.dn.net");
	response = RRPAddDomain("12345thisisatest.com", nameServers, -1);
	RRPFreeVector(nameServers);

	if (response == NULL) {
		RRPPrintInternalErrorDescription();
		exit(1);
	}
	else if (response->code != 200) {
		printf("Error registering domain name: %s\n", response->description);
		exit(1);
	}

	RRPFreeResponse(response);
	printf("Domain registered successfully\n");

	/*
	** Register new name server
	*/
	ipAddresses = RRPCreateVector();
	RRPAddVectorElement(ipAddresses, "203.23.21.5");
	response = RRPAddNameServer("ns1.12345thisisatest.com", ipAddresses);
	RRPFreeVector(ipAddresses);

	if (response == NULL) {
		RRPPrintInternalErrorDescription();
		exit(1);
	}
	else if (response->code != 200) {
		printf("Error registering nameserver: %s\n", response->description);
		exit(1);
	}

	RRPFreeResponse(response);
	printf("Name server registered successfully\n");

	/*
	** end session
	*/
	response = RRPEndSession();

	if (response == NULL) {
		RRPPrintInternalErrorDescription();
		exit(1);
	}

	RRPFreeResponse(response);
	printf("Closing connection\n");

	exit(0);

} /* main() */
