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



/**
 * InvalidArgumentException
 *
 * @author Stephen Mahlstedt
 * @version 1.0 1/9/1999
 */

public class InvalidArgumentException extends RRPException {

	public InvalidArgumentException () {
		super();
	} // constructor()



	public InvalidArgumentException (String message) {
		super(message);
	} // constructor()



	public InvalidArgumentException (String message, Throwable x) {
		super(message, x);
	} // constructor()



	public String getMessage () {
		return super.getMessage();
	} // getMessage()



} // InvalidArgumentException
