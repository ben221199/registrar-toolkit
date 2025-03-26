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

package nsi.registry.client.java.rrp.api;



import java.io.*;
import java.net.*;


/**
 * SecureSocket provides a common interface to a socket implementation.
 * This object can be implemented using any socket package as long as
 * the public method signatures defined in <code>SecureSocketBase</code>
 * are implemented.
 *
 * By default this object
 * is implemented using the java.net package.
 *
 * @author   Stephen Mahlstedt
 * @version  1.0 01/08/1999
 * @see      SecureSocketBase
 */
public class SecureSocket extends SecureSocketBase {


	/**
	 * Creates a stream socket and connects it to the specified port
	 * number on the named host. 
	 *
	 * @param host  the host name.
	 *
	 * @param port  the port number.
	 *
	 * @exception IOException if an I/O error occurs when creating
	 *                        the socket.
	 *
	 * @exception UnknownHostException if the IP address of a host could
	 *                                 not be determined.
	 */
	public SecureSocket (String host, int port) throws
		UnknownHostException, IOException {

		//
		// Create and store instance of socket
		//
		// Currently creates an instance of java.net.Socket
		//
		// *** This will need to be substituted with a secure socket
		//	object to meet the RRPServer security requirements *** 
		//
		//
		setSocket(new Socket(host, port));

	} // constructor()




	/**
	 * Returns an input stream for this socket.
	 *
	 * @return an input stream for reading bytes from this socket.
	 *
	 * @exception IOException if an I/O error occurs when creating the
	 *                        input stream.
	 */
	public InputStream getInputStream () throws IOException {
		return getSocket().getInputStream();
	} // getInputStream()




	/**
	 * Returns an output stream for this socket.
	 *
	 * @return an output stream for writing bytes to this socket.
	 *
	 * @exception IOException if an I/O error occurs when creating the
	 *                        output stream.
	 */
	public OutputStream getOutputStream () throws IOException {
		return getSocket().getOutputStream();
	} // getOutputStream()



	/**
	 * Returns the address to which the socket is connected
	 *
	 * @return the remote IP address to which this socket is connected.
	 */
	public InetAddress getInetAddress () {
		return getSocket().getInetAddress();
	} // getInetAddress()





	/**
	 * Enable/disable SO_TIMEOUT with the specified timeout, in
	 * milliseconds. With this option set to a non-zero timeout,
	 * a read() call on the InputStream associated with this Socket
	 * will block for only this amount of time. If the timeout expires,
	 * a java.io.InterruptedIOException is raised, though the Socket
	 * is still valid. The option must be enabled prior to entering the
	 * blocking operation to have effect. The timeout must be > 0. A
	 * timeout of zero is interpreted as an infinite timeout. 
	 */
	public void setSoTimeout (int timeout) throws SocketException {
		getSocket().setSoTimeout(timeout);
	} // setSoTimeout()






	/**
	 * Closes this socket.
	 *
	 * @exception IOException if an I/O error occurs when closing this socket.
	 */
	public void close () throws IOException {
		getSocket().close();
	} // close()





	//
	// Currently returns an instance of java.net.Socket
	//
	// *** This will need to be substituted with a secure socket
	//	object to meet the RRPServer security requirements *** 
	//
	private Socket getSocket () {
		return socket;
	} // getSocket()




	//
	// Currently takes an instance of java.net.Socket
	//
	// *** This will need to be substituted with a secure socket
	//	object to meet the RRPServer security requirements *** 
	//
	private void setSocket (Socket aSocket) {
		socket = aSocket;
	} // setSocket()




	//
	// Currently, an instance of java.net.Socket
	//
	// *** This will need to be substituted with a secure socket
	//	object to meet the RRPServer security requirements *** 
	//
	private Socket socket = null;


} // SecureSocket
