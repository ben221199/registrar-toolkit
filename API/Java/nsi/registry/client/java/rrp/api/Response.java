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



import java.util.NoSuchElementException;
import java.util.StringTokenizer;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;



/**
 * This class represents the various components of an RRP response.
 *
 * @author     Stephen Mahlstedt
 * @version    1.0 01/08/1999
 */

public class Response {

	/**
	 * Parses an RRP text response and extracts and stores the
	 * various components.
	 *
	 * @param data  an RRP text response.
	 *
	 * @exception InvalidArgumentException  if <code>data</code> is null.
	 *
	 * @exception InvalidResponseFormatException  if <code>data</code> is
	 *                                            not a properly formatted
	 *                                            RRP response.
	 */
	public Response (String data) throws InvalidArgumentException,
	InvalidResponseFormatException {

		if (data == null) {
			throw new InvalidArgumentException("null argument: data");
		}

		parseRawResponse(data);
		setRawResponse(data);

	} // constructor





	public static void displayResponse (Response response) {
		System.out.println("Code: " + response.getCode());
		System.out.println("Description: " + response.getDescription());

		Hashtable attributes = response.getAttributes();

		if (attributes == null) {
			System.out.println("Attributes: null");
			return;
		}

		System.out.println("Attributes:");

		Enumeration enum = attributes.keys();

		while (enum.hasMoreElements()) {
			String key = null;
			Vector vector = null;

			key = (String) enum.nextElement();
			vector = (Vector) attributes.get(key);

			for (int i=0; i < vector.size(); i++) {
				System.out.println(key + ":" + (String)vector.elementAt(i));
			}
		}

	} // displayResponse()


	/**
	 * Returns the original RRP text response string used to instantiate
	 * object.
	 *
	 * @return the original RRP text response.
	 */
	public String getRawResponse () {
		return rawResponse;
	} // getRawResponse





	/**
	 * Returns the RRP response code extracted from the original RRP
	 * text response.
	 *
	 * @return the RRP response code.
	 */
	public int getCode () {
		return code;
	} // getCode()





	/**
	 * Returns the RRP response description extracted from the original RRP
	 * text response.
	 *
	 * @return the RRP response description.
	 */
	public String getDescription () {
		return description;
	} // getDescription()






	/**
	 * Returns any attributes, if applicable, extracted from the original
	 * RRP text response.
	 *
	 * @return response attributes if any exist, null otherwise.
	 */
	public Hashtable getAttributes () {
		return attributes;
	} // getAttributes()






	//
	// Parse RRP text response and extract and store all components
	//
	private void parseRawResponse (String data) throws
	InvalidResponseFormatException {

		//
		// Split data by '\r\n'
		//
		StringTokenizer st = new StringTokenizer(data, "\r\n");


		//
		// Get the first line of the response
		//
		String responseLine = "";

		try {
			responseLine = st.nextToken();

		} catch (NoSuchElementException e) {
			throw new InvalidResponseFormatException("Missing first line");
		}


		//
		// Extract "result code" and "description" from first line
		// of response.
		//

		int spaceIndex = responseLine.indexOf(' ');

		//
		// If there is no space in the string, then the format is invalid.
		// First line must be "code description"
		//
		if (spaceIndex == -1) {
			throw new InvalidResponseFormatException(
			"missing code and description");
		}

		//
		// The code is comprised of the set of characters spanning the
		// the first character to the space
		//
		String resultCode = responseLine.substring(0, spaceIndex);

		//
		// trim any whitespace from both ends of code
		//
		resultCode = resultCode.trim();

		//
		// Attempt to convert the string code to an integer
		//
		try {
			setCode(Integer.parseInt(resultCode));
		}
		catch (NumberFormatException e) {
			throw new InvalidResponseFormatException(
				"Missing response code");
		}




		//
		// Extract the descrption now
		//
		String desc = null;

		//
		// The description is comprised of the set of characters spanning from
		// the first character after the space to the end of the entire string
		//
		desc = responseLine.substring(spaceIndex + 1, responseLine.length());

		//
		// trim any whitespace from both ends of code
		//
		desc = desc.trim();

		if (desc.length() == 0) {
			throw new InvalidResponseFormatException(
				"Missing description");
		}

		setDescription(desc);




		String key;
		String value;
		int colonIndex = 0;

		//
		// Iterate through the rest of the response, searching for
		// attributes. Break from the loop when we encounter the
		// response terminator
		//
		try {
			while (st.hasMoreTokens() ) {
				//
				// Get the next line from the response
				//
				responseLine = st.nextToken();

				//
				// Try to find the colon (':') in the string
				//
				colonIndex = responseLine.indexOf(':');

				//
				// If the colon is missing, then check to see if we've
				// encountered the response terminator. If we haven't,
				// then the response is formatted improperly
				//
				if (colonIndex == -1) {
					responseLine = responseLine.trim();

					//
					// Break from the loop if we hit the response terminator
					//
					if (responseLine.equals(".")) {
						break;
					}
					else {
						throw new InvalidResponseFormatException(
							"Invalid attribute key-value pair");
					}
				}
					
				//
				// The attribute key is everything before the colon
				//
				key = responseLine.substring(0, colonIndex);
				key = key.trim();

				//
				// The attribute value is everything after the colon
				//
				value = responseLine.substring(colonIndex + 1);
				value = value.trim();

				//
				// Save attribute
				//
				addAttribute(key.toLowerCase(), value);

			}

		}
		catch (NoSuchElementException e) {
			throw new InvalidResponseFormatException(
				"Error splitting response by '\r\n'", e);
		}


	} // parseRawData()





	//
	// Attributes are stored in a hashtable. Since there can be multiple
	// values for the same key, we store all values in vectors. Everytime
	// a value is added for a key that already exists, we just add that
	// value to that key's vector. If a key does not yet exist, then
	// we create a new vector and add the value to it.
	//
	private void addAttribute (String key, String value) {
		Hashtable hash = getAttributes();

		if (hash == null) {
			hash = new Hashtable();
			setAttributes(hash);
		}

		Vector vector = (Vector) hash.get(key);

		if (vector == null) {
			vector = new Vector(1);
			hash.put(key, vector);
		}
	
		vector.addElement(value);

	} // addAttribute()





	private void setRawResponse (String aRawResponse) {
		rawResponse = aRawResponse;
	} // setRawResponse




	private void setCode (int aCode) {
		code = aCode;
	} // setCode()




	private void setDescription (String aDescription) {
		description = aDescription;
	} // setDescription()



	private void setAttributes (Hashtable hash) {
		attributes = hash;
	} // setAttributes()






	//
	// The original RRP text response string used to instantiate object.
	//
	private String rawResponse = null;


	//
	// The RRP response code extracted from the original RRP text response.
	//
	private int code;


	//
	// The RRP response description extracted from the original RRP
	// text response.
	//
	private String description = null;


	//
	// The RRP response attributes extracted from the original RRP
   // text response.
	//
	private Hashtable attributes = null;


} // Response
