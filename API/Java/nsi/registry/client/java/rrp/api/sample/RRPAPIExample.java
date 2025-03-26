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

import nsi.registry.client.java.rrp.api.*;
import java.util.Vector;



public class RRPAPIExample {


	public static void main (String argv[]) {

		Response response = null;
		RRPSession session = null;


		//
		// Connect to RRP server and create an authenticated session
		//
		try {
			session = new RRPSession("sampleserver", 648);

			response = session.startSession("registrarId", 
				"registrarPassword", null);	

			if (response.getCode() != 200) {
				System.err.println("RRP Authentication error");
				System.exit(1);
			}
				
		}
		catch (RRPException e) {
			System.err.println("Error creating RRP session: " +
				e.getMessage());
			System.exit(1);
		}




		//
		// Register a new domain name
		//
		try {
			Vector nameServers = new Vector();

			nameServers.addElement("NS-01A.ANS.NET");
			nameServers.addElement("NS-02A.ANS.NET");
			response = session.addDomain("rrpexampledomain.com", nameServers);	

			if (response.getCode() != 200) {
				System.err.println("Error registering domain name: " +
					response.getDescription());
				System.exit(1);
			}
		}
		catch (RRPException e) {
			System.err.println("Error registering domain name: " +
				e.getMessage());
			System.exit(1);
		}




		//
		// Register NameServer
		//
		try {
			Vector ipAddresses = new Vector();

			ipAddresses.addElement("199.33.33.55");
			response = session.addNameServer("ns1.rrpexampledomain",
				ipAddresses);	

			if (response.getCode() != 200) {
				System.err.println("Error registering nameserver: " +
					response.getDescription());
				System.exit(1);
			}
		}
		catch (RRPException e) {
			System.err.println("Error registering nameserver: " +
				e.getMessage());
			System.exit(1);
		}


	} // main

} // RRPAPIExample
