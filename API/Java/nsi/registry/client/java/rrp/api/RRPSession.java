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

package nsi.registry.client.java.rrp.api;



import java.io.*;
import java.net.*;
import java.util.Vector;
import java.util.Properties;
import java.util.Enumeration;




/**
 * RRPSession provides a simple interface for connecting and issuing,
 * commands to, and receiving RRP responses from an RRP server. 
 *
 * @author     Stephen Mahlstedt
 * @version    1.0 01/08/1999
 *
 * @modified by Lihui Zhang
 * @version    1.1 10/26/1999
 * 
 * Two interfaces were implemented for version 1.1, 
 * 	addDomain with registration period and 
 * 	renewDomain with renew registration period
 * 
 * @modified by Lihui Zhang
 * @version    1.1 11/22/1999
 * 
 * Add another 'optional option' currentExpirationYear to renewDomain 
 * interface. 
 */

public class RRPSession {

	/**
	 * Creates an unauthenticated RRP session by connecting to
	 * the specified RRP Server process.
	 *
	 * @param host  hostname of RRP server with which to connect
	 *
	 * @param port  RRP server port to connect to
	 *
	 * @exception RRPException  if <code>host</code> is null or empty,
	 *                          if <code>port</code> less than zero or
	 *                          if any internal errors occur
	 */
	public RRPSession (String host, int port)
	throws RRPException {
		//
		// Validate constructor input
		//
		if (host == null) {
			throw new RRPException("null argument: host");
		}
		else if (host.length() == 0) {
			throw new RRPException("invalid argument: host");
		}
		else if (port < 0) {
			throw new RRPException("invalid argument: port");
		}


		//
		// Try to create new SecureSocket
		//
		try {
			setSocket(new SecureSocket(host, port));
		}
		catch (UnknownHostException e) {
			throw new RRPException("Cannot create socket connection", e);
		}
		catch (IOException e) {
			throw new RRPException("Cannot create socket connection", e);
		}


		//
		// Attempt to read greeting from server
		//
		try {
			readRRPResponse();
		}
		catch (Exception e) {
			throw new RRPException("Error reading server greeting", e);
		}

	} // constructor()





	/**
	 * Creates an unauthenticated RRP session by connecting to
	 * the specified RRP Server process.
	 *
	 * @param host  hostname of RRP server with which to connect
	 *
	 * @param port  RRP server port to connect to
	 *
	 * @param aTimeout a timeout value (in seconds) for socket operations.
	 *                 A value of 0 disable timeout timer
	 *
	 * @exception RRPException  if <code>host</code> is null or empty,
	 *                          if <code>port</code> less than zero or
	 *                          if any internal errors occur
	 */
	public RRPSession (String host, int port, long aTimeout)
	throws RRPException {

		this(host, port);

		//
		// Validate constructor input
		//
		if (aTimeout < 0) {
			throw new RRPException("invalid argument: aTimeout");
		}

		//
		// Store timeout so we can use it later on
		//
		setTimeout(aTimeout * 1000);
		if (aTimeout > 0) {
			try {
				getSocket().setSoTimeout(new Long(getTimeout()).intValue());
			}
			catch (Exception e) {
				throw new RRPException("Error setting socket timeout", e);
			}
		}	

	} // constructor()






	/**
	 * This method enables a registrar to authenticate an RRP connection.
	 * This method must be called successfully prior to issuing any other
	 * RRP requests.
	 *
	 * @param id  ID of the registrar
	 *
	 * @param password  Password of the registrar
	 *
	 * @param newPassword An optional new password to set for the
	 *                    registrar. A null value can be passed if
	 *                    parameter is not applicable.
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 *
	 * @exception RRPException  if <code>id</code> or <code>password</code>
	 *                          is null or if any internal errors occur
	 */
	public Response startSession (String id, String password,
	String newPassword) throws RRPException {

		//
		// Validate arguments
		//
		if (id == null) {
			throw new RRPException("null argument: id");
		}
		else if (password == null) {
			throw new RRPException("null argument: password");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Session\r\n");
		buf.append("-Id:" + id + "\r\n");
		buf.append("-Password:" + password + "\r\n");

		if (newPassword != null) {
			buf.append("-NewPassword:" + newPassword + "\r\n");
		}
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // startSession()





	/**
	 * Enables a registrar to register a domain name
	 *
	 * @param domainName  A fully qualified domain name to register
	 *
	 * @param nameServers  An optional vector containing the fully
	 *                     qualified host names of the registered
	 *                     name servers that will be hosting
	 *                     <code>domainName</code>
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null,
	 *                          or if any internal errors occur
	 */
	public Response addDomain (String domainName, Vector nameServers)
	throws RRPException {
		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Add\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");

		if (nameServers != null) {
			String nameServer = null;

			for (int i=0; i < nameServers.size(); i++) {
				nameServer = (String) nameServers.elementAt(i);
				buf.append("NameServer:" + nameServer + "\r\n");
			}
		}

		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

		
	} // addDomain()



	/**
	 * Enables a registrar to register a domain name with an explicit 
    * registration period for RRP Version 1.1
	 *
    * @author Lihui Zhang 
    *
	 * @param domainName  A fully qualified domain name to register
	 * @param nameServers  An optional vector containing the fully
	 *                     qualified host names of the registered
	 *                     name servers that will be hosting
	 *                     <code>domainName</code>
	 * @param registrationPeriod A new option for version 1.1, 
    *									  stands for the registration period of
	 *									  the domain to be added.
    *										
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null,
	 *                          or if any internal errors occur
	 */
	public Response addDomain (String domainName, Vector nameServers, 
									   int registrationPeriod)
	throws RRPException {
		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Add\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");

		if (nameServers != null) {
			String nameServer = null;

			for (int i=0; i < nameServers.size(); i++) {
				nameServer = (String) nameServers.elementAt(i);
				buf.append("NameServer:" + nameServer + "\r\n");
			}
		}

      buf.append("-Period:" + registrationPeriod + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

		
	} // addDomain()




	/**
	 * Enables a registrar to register a name server
	 *
	 * @param nameServer  A fully qualified host name of a name server
	 *
	 * @param ipAddresses  An vector containing one or more
	 *                     IP addresses of the name server
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>nameServer</code> or
	 *                          <code>ipAddresses</code> is null,
	 *                          or if any internal errors occur
	 */
	public Response addNameServer (String nameServer, Vector ipAddresses)
	throws RRPException {
		//
		// Validate arguments
		//
		if (nameServer == null) {
			throw new RRPException("null argument: nameServer");
		}
		else if (ipAddresses == null) {
			throw new RRPException("null argument: ipAddresses");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Add\r\n");
		buf.append("EntityName:NameServer\r\n");
		buf.append("NameServer:" + nameServer + "\r\n");

		if (ipAddresses != null) {
			String ipAddress = null;

			for (int i=0; i < ipAddresses.size(); i++) {
				ipAddress = (String) ipAddresses.elementAt(i);
				buf.append("IpAddress:" + ipAddress + "\r\n");
			}
		}

		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());
		
	} // addNameServer()





	/**
	 * Enables a registrar to check the availability of a domain name
	 *
 	 * @param domainName  A fully qualified domain name to check
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response checkDomain (String domainName)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Check\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // checkDomain()






	/**
	 * Enables a registrar to check the availability of a name server
	 *
 	 * @param nameServer  The hostname of the name server to check
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>nameServer</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response checkNameServer (String nameServer)
	throws RRPException {

		//
		// Validate arguments
		//
		if (nameServer == null) {
			throw new RRPException("null argument: nameServer");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Check\r\n");
		buf.append("EntityName:nameserver\r\n");
		buf.append("NameServer:" + nameServer + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // checkNameServer()






	/**
	 * Enables a registrar to delete the registration of a domain name
	 *
 	 * @param domainName  A fully qualified domain name to delete
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response deleteDomain (String domainName)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Del\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");
		buf.append(".\r\n");

		return processRequest(buf.toString());

	} // deleteDomain()







	/**
	 * Enables a registrar to delete the registration of a name server
	 *
 	 * @param nameServer  Fully qualified host name of name server to delete
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  If <code>nameServer</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response deleteNameServer (String nameServer)
	throws RRPException {
		//
		// Validate arguments
		//
		if (nameServer == null) {
			throw new RRPException("null argument: nameServer");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Del\r\n");
		buf.append("EntityName:NameServer\r\n");
		buf.append("NameServer:" + nameServer + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // deleteNameServer()





	/**
	 * Enables a registrar to get general information about RRP
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @param target  Optional string indiciating the type of information
	 *                desired. The default value if <code>target</code> is
	 *                null is "Protocol"
	 *
	 * @exception RRPException  If any internal errors occur
	 */
	public Response describe (String target) throws RRPException {

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Describe\r\n");

		//
		// Target option is optional
		//
		if (target != null) {
			buf.append("-Target:" + target + "\r\n");
		}
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // describe()






	/**
	 * Enables a registrar to update a registered domain name
	 *
 	 * @param domainName  A fully qualified domain name to update
	 *
	 *
	 * @param addedNameServers  A Vector of registered name servers
	 *                          to add to the domain
	 *
	 * @param modifiedNameServers  A Properties object containing
	 *                             the domain's name servers to replace.
	 *                             The <code>key</code> is the old name server
	 *                             and the <code>value</code> is the new
	 *
	 * @param deletedNameServers  A Vector of registered name servers
	 *                            to delete from the domain
	 *
	 *
	 *
	 * @param addedStatuses  A Vector of statuses to add to the domain
	 *
	 * @param modifiedStatuses  A Properties object containing
	 *                             the domain's statuses to replace.
	 *                             The <code>key</code> is the old status
	 *                             and the <code>value</code> is the new
	 *
	 * @param deletedStatuses  A Vector of statuses to delete from the domain
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response modifyDomain (String domainName,
	Vector addedNameServers, Properties modifiedNameServers,
	Vector deletedNameServers, Vector addedStatuses,
	Properties modifiedStatuses, Vector deletedStatuses) throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domain");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Mod\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");

	

		//
		// Build "added name servers" portion of command
		//
		if (addedNameServers != null) {
			String server = null;

			for (int i=0; i < addedNameServers.size(); i++) {
				server = (String) addedNameServers.elementAt(i);
				buf.append("NameServer:" + server + "\r\n");
			}
		}


		//
		// Build "modified name servers" portion of command
		//
		if (modifiedNameServers != null) {
			Enumeration enum = modifiedNameServers.keys();
			String oldServer = null;
			String newServer = null;

			while (enum.hasMoreElements()) {
				oldServer = (String) enum.nextElement();
				newServer = modifiedNameServers.getProperty(oldServer);

				buf.append("NameServer:" + oldServer + "=" + newServer + "\r\n");
			}
		}


		//
		// Build "deleted name servers" portion of command
		//
		if (deletedNameServers != null) {
			String server = null;

			for (int i=0; i < deletedNameServers.size(); i++) {
				server = (String) deletedNameServers.elementAt(i);
				buf.append("NameServer:" + server + "=\r\n");
			}
		}


		//
		// Build "added statuses" portion of command
		//
		if (addedStatuses != null) {
			String status = null;

			for (int i=0; i < addedStatuses.size(); i++) {
				status = (String) addedStatuses.elementAt(i);
				buf.append("Status:" + status + "\r\n");
			}
		}


		//
		// Build "modified statuses" portion of command
		//
		if (modifiedStatuses != null) {
			Enumeration enum = modifiedStatuses.keys();
			String oldStatus = null;
			String newStatus = null;

			while (enum.hasMoreElements()) {
				oldStatus = (String) enum.nextElement();
				newStatus = modifiedStatuses.getProperty(oldStatus);

				buf.append("Status:" + oldStatus + "=" + newStatus + "\r\n");
			}
		}


		//
		// Build "deleted statuses" portion of command
		//
		if (deletedStatuses != null) {
			String status = null;

			for (int i=0; i < deletedStatuses.size(); i++) {
				status = (String) deletedStatuses.elementAt(i);
				buf.append("Status:" + status + "=\r\n");
			}
		}



		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());
		
	} // modifyDomain()






	/**
	 * Enables a registrar to update a registered name server
	 *
 	 * @param nameServer  A fully qualified host name of a registered
	 *                    name server to update
	 *
	 * @param addedIPAddresses  A Vector of IP addresses to add to the
	 *                          name server
	 *
	 * @param modifiedIPAddresses  A Properties object containing
	 *                             the name servers's IP addresses with which
	 *                             to replace. Each <code>key</code> is an 
	 *                             old IP address and the <code>value</code>
	 *                             is the new IP Address
	 *
	 * @param deletedIPAddresses  A Vector of IP addresses to delete from
	 *                            the name server
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>nameServer</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response modifyNameServer (String nameServer,
	String newNameServer, Vector addedIPAddresses,
	Properties modifiedIPAddresses, Vector deletedIPAddresses)
	throws RRPException {

		//
		// Validate arguments
		//
		if (nameServer == null) {
			throw new RRPException("null argument: nameServer");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Mod\r\n");
		buf.append("EntityName:NameServer\r\n");
		buf.append("NameServer:" + nameServer + "\r\n");

		//
		// Add "new name server" to command if one was provided
		//
		if (newNameServer != null) {
			buf.append("NewNameServer:" + newNameServer + "\r\n");
		}
		
	
		//
		// Build "added IP addresses" portion of command
		//
		if (addedIPAddresses != null) {
			String server = null;

			for (int i=0; i < addedIPAddresses.size(); i++) {
				server = (String) addedIPAddresses.elementAt(i);
				buf.append("IPAddress:" + server + "\r\n");
			}
		}


		//
		// Build "modified IP addresses" portion of command
		//
		if (modifiedIPAddresses != null) {
			Enumeration enum = modifiedIPAddresses.keys();
			String oldServer = null;
			String newServer = null;

			while (enum.hasMoreElements()) {
				oldServer = (String) enum.nextElement();
				newServer = modifiedIPAddresses.getProperty(oldServer);

				buf.append("IPAddress:" + oldServer + "=" + newServer + "\r\n");
			}
		}


		//
		// Build "deleted IP addresses" portion of command
		//
		if (deletedIPAddresses != null) {
			String server = null;

			for (int i=0; i < deletedIPAddresses.size(); i++) {
				server = (String) deletedIPAddresses.elementAt(i);
				buf.append("IPAddress:" + server + "=\r\n");
			}
		}


		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());
		
	} // modifyNameServer()






	/**
	 * Enables a registrar to renew the registration of a domain name
	 *
 	 * @param domainName  A fully qualified domain name to renew
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response renewDomain (String domainName)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Renew\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // renewDomain()


	/**
	 * Enables a registrar to renew the registration of a domain name
	 *
 	 * @author Lihui Zhang
	 *
 	 * @param domainName  A fully qualified domain name to renew
 	 * @param renewRegistrationPeriod
	 *							 The renew registration period of the domain
 	 * @param currentExpirationYear 
	 *							 The current expiration year. If accepted, the
    *							 domain will be renewed starting from this year.
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */

	public Response renewDomain (String domainName, 
										  int renewRegistrationPeriod, 
										  int currentExpirationYear)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}


		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Renew\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");

      buf.append("-Period:" + renewRegistrationPeriod + "\r\n");
      buf.append("-CurrentExpirationYear:" + currentExpirationYear + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // renewDomain()







	/**
	 * Enables a registrar to query a domain name. The following attributes
	 * will be included in a successful <code>Response</code>:
	 *
	 * <UL>
	 * <LI>Domain name
	 * <LI>Name servers
	 * <LI>Registration expiration date
	 * <LI>ID of the current registrar
	 * <LI>Date the domain name was transferred by the current registrar
	 * <LI>Current status of the domain
	 * <LI>Date the domain name was registered
	 * <LI>ID of the registrar that registered the domain name
	 * <LI>Date the domain name was last updated
	 * <LI>ID of the entity that last updated the domain
	 * </UL>
	 *
 	 * @param domainName  A fully qualified domain name to query
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response statusDomain (String domainName)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		StringBuffer buf = new StringBuffer();

		buf.append("Status\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");
		buf.append(".\r\n");

		return processRequest(buf.toString());

	} // statusDomain()






	/**
	 * Enables a registrar to query a name server. The following
	 * attributes will be included in a successful <code>Response</code>:
	 *
	 * <UL>
	 * <LI>Fully qualified host name of name server
	 * <LI>IP addresses of the name server
	 * <LI>ID of the current registrar
	 * <LI>Date the name server was transferred by the current registrar
	 * <LI>Date the name server was registered
	 * <LI>ID of the entity that registered the name server
	 * <LI>Date the name server was last updated
	 * <LI>ID of the entity that that last updated the name server
	 * </UL>
	 *
	 * @param nameServer  Fully qualified host name of name server to query.
	 *
	 * @return  a Response object containing the response code, description
	 *			 and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  If <code>nameServer</code> is null
	 *								  or if any internal errors occur
	 */
	public Response statusNameServer (String nameServer)
	throws RRPException {
		//
		// Validate arguments
		//
		if (nameServer == null) {
			throw new RRPException("null argument: nameServer");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Status\r\n");
		buf.append("EntityName:NameServer\r\n");
		buf.append("NameServer:" + nameServer + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//
		return processRequest(buf.toString());

	} // statusNameServer()




	/**
	 * Enables a registrar to transfer a domain name from another
	 * registrar to itself
	 *
 	 * @param domainName  A fully qualified domain name to transfer
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response transferDomain (String domainName)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Transfer\r\n");
		buf.append("EntityName:Domain\r\n");
		buf.append("DomainName:" + domainName + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // transferDomain()







	/**
	 * Enables a registrar to approve or deny the transfer of a domain
	 * from itself to another registrar
	 *
 	 * @param domainName  A fully qualified domain name to transfer
	 *
 	 * @param approve  <code>true</code> to approve the transfer,
	 *                 <code>false</code> to deny it.
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if <code>domainName</code> is null 
	 *                          or if any internal errors occur
	 */
	public Response transferDomain (String domainName, boolean approve)
	throws RRPException {

		//
		// Validate arguments
		//
		if (domainName == null) {
			throw new RRPException("null argument: domainName");
		}

		//
		// Build the raw RRP request
		//
		StringBuffer buf = new StringBuffer();

		buf.append("Transfer\r\n");
		buf.append("EntityName:Domain\r\n");

		if (approve == true) {
			buf.append("-Approve:Yes\r\n");
		}
		else {
			buf.append("-Approve:No\r\n");
		}

		buf.append("DomainName:" + domainName + "\r\n");
		buf.append(".\r\n");

		//
		// Process the request and return the response
		//	
		return processRequest(buf.toString());

	} // transferDomain()






	/**
	 * Enables a registrar to close an RRP connection
	 *
	 * @return  a Response object containing the response code, description
	 *          and attributes - if applicable - returned from the RRP request
	 *
	 * @exception RRPException  if any internal errors occur
	 */
	public Response endSession () throws RRPException {

		//
		// Process the request and return the response
		//	
		return processRequest("Quit\r\n.\r\n");

	} // endSession()









	//
	// Sends the request to the protocol server, reads the text
	// RRP response and attempts to create and return a new
	// Response object using the raw text response
	//
	private Response processRequest (String data) throws RRPException {

		sendRRPRequest(data);
		String response = readRRPResponse();

		try {
			return new Response(response);
		}
		catch (Exception e) {
			throw new RRPException("Error processing request", e);
		}

	} // processRequest()






	//
	// Sends raw RRP command to RRP Server and flushes stream
	//
	private void sendRRPRequest (String data) throws RRPException {

		//
		// Validate parameters
		//
		if (data == null) {
			throw new RRPException("null parameter: data");
		}

		try {
			BufferedWriter writer = getBufferedWriter();

			writer.write(data);
			writer.flush();
		}
		catch (IOException e) {
			throw new RRPException("Error writing data to socket", e);
		}

	} // sendRRPRequest()






	//
	// Reads an RRP response from socket input stream.
	//
	// Iterates through the byte array populated by the 'read' command
	// and adds each character to a StringBuffer. Once the buffer
	// ends with an RRP response terminator ('\r\n.\r\n'), then the
	// buffer is converted to a string and is returned
	//
	private String readRRPResponse () throws RRPException {
		//
		// used to keep track of how many bytes were read in from stream
		// for each returned call to read() method
		//
		int  byteCount =  0;

		//
		// Array used to temporarily store data read from socket
		//
		byte bytes[] =  new byte[BLKSIZE];

		//
		// Get BufferedInputStream from socket's inputStream.
		// This will allow us to read more efficiently from the
		// socket
		//
		BufferedInputStream inputStream = null;

		try {
			inputStream = getBufferedInputStream();
		}
		catch (IOException e) {
			throw new RRPException("Error creating output stream", e);
		}


		char inputChar;
		StringBuffer responseBuf = new StringBuffer();


		//
		// Read in data from socket until a response terminator is
		// encountered, or an error occurs
		//

		if (getTimeout() > 0) {
			setTimeoutCounter(System.currentTimeMillis());
		}

		while (true) {


			//
			// Attempt to read in a chunk of data from the socket
			//
			try {
				byteCount = inputStream.read(bytes, 0, BLKSIZE);
				setTimeoutCounter(0);
				//
				// check to make sure no errors occured while reading
				//
				if (byteCount <= 0) {
					throw new RRPException("Error reading from socket");
				}
			}
			catch (InterruptedIOException e) {
				//
				// Check for timeouts
				//
				if (getTimeout() > 0 && getTimeoutCounter() > 0) {
					long diff;
					diff  = System.currentTimeMillis() - getTimeoutCounter();
					if (diff > getTimeout()) {
						setTimeoutCounter(0);
						throw new RRPException("Socket operation timeout");
					}
				}
				continue;
			}
			catch (IOException e) {
				throw new RRPException("Error reading from socket", e);
			}


			//
			// Now that we have a chunk of data to work with, go through
			// it and start building a raw RRP response. When a reponse
			// terminator is encountered, return response.
			//
			for (int i=0; i < byteCount; i++) {
				//
				// Append character to command buffer
				//
				inputChar = (char) bytes[i];

				responseBuf.append(inputChar);


				//
				// Only check for a response terminator if character
				// is '\n'. We don't want to bother checking the end
				// of the StringBuffer unless necessary
				//
				if (inputChar == '\n')  {
					String response = responseBuf.toString();

					if (response.endsWith("\r\n.\r\n")) {
						return response;
					}
				}

			} // for

			setTimeoutCounter(System.currentTimeMillis());

		} // while


	} // readRRPResponse()






	private BufferedInputStream getBufferedInputStream () 
	throws IOException {

		//
		// If bufferedInputStream doesn't exist yet, then
		// create it and store it
		//
		if (null == bufferedInputStream) {

			DataInputStream dataIStream = null;
			BufferedInputStream bufferedIStream = null;

			dataIStream = new DataInputStream(getSocket().getInputStream());
			bufferedIStream = new BufferedInputStream(dataIStream, BLKSIZE);

			setBufferedInputStream(bufferedIStream);
		}

		return bufferedInputStream;

	} // getBufferedInputStream()





	//
	// Returns the bufferedWriter instance, if it exists. If it
	// doesn't yet exist, create it using the socket output stream,
	// store it and return it
	//
	private BufferedWriter getBufferedWriter () throws IOException {

		//
		// If bufferedWriter doesn't exist, create it and store it
		//
		if (bufferedWriter == null) {

			OutputStream outStream = getSocket().getOutputStream();

			BufferedWriter writer;
			writer= new BufferedWriter(new OutputStreamWriter(outStream));

			setBufferedWriter(writer);
		}

		return bufferedWriter;

	} // getBufferedWriter()





	private SecureSocket getSocket () {
		return socket;
	} // getSocket()




	private long getTimeout () {
		return timeout;
	} // getTimeout()




	private long getTimeoutCounter () {
		return timeoutCounter;
	} // getTimeoutCounter()







	private void setSocket (SecureSocket aSocket) {
		socket = aSocket;
	} // setSocket()




	private void setBufferedInputStream (BufferedInputStream bis) {
		bufferedInputStream = bis;
	} // setBufferedInputStream()




	private void setBufferedWriter (BufferedWriter pw) {
		bufferedWriter = pw;
	} // setBufferedWriter()




	private void setTimeout (long aValue) {
		timeout = aValue;
	} // setTimeout()



	private void setTimeoutCounter (long aValue) {
		timeoutCounter = aValue;
	} // setTimeoutCounter()





	//
	// IO stream used to read from client socket
	//
	private BufferedInputStream bufferedInputStream = null;



	//
	// Stream used to write data to socket
	//
	private BufferedWriter bufferedWriter = null;



	//
	// Socket connection to RRP server
	//
	private SecureSocket socket = null;



	//
	// Socket timeout setting (in seconds). A value of 0 disables the timer
	//
	private long timeout = 0;



	//
	// Socket timeout counter
	//
	private long timeoutCounter = 0;




	//
	// Size of input buffer to use when reading data from socket
	//
	private final int BLKSIZE = 256;



} // RRPSession
