rem ############################################################################
rem # Copyright (C) 2000 Network Solutions, Inc.
rem #
rem # This library is free software; you can redistribute it and/or
rem # modify it under the terms of the GNU Lesser General Public
rem # License as published by the Free Software Foundation; either
rem # version 2.1 of the License, or (at your option) any later version.
rem #
rem # This library is distributed in the hope that it will be useful,
rem # but WITHOUT ANY WARRANTY; without even the implied warranty of
rem # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
rem # Lesser General Public License for more details.
rem #
rem # You should have received a copy of the GNU Lesser General Public
rem # License along with this library; if not, write to the Free Software
rem # Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
rem #
rem # Network Solutions, Inc. Registry
rem # 505 Huntmar Park Dr.
rem # Herndon, VA 20170
rem ############################################################################
rem # The RRP, APIs and Software are provided "as-is" and without any warranty #
rem # of any kind.  NSI EXPRESSLY DISCLAIMS ALL WARRANTIES AND/OR CONDITIONS,  #
rem # EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES#
rem # AND CONDITIONS OF MERCHANTABILITY OR SATISFACTORY QUALITY AND FITNESS FOR#
rem # A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  NSI DOES#
rem # NOT WARRANT THAT THE FUNCTIONS CONTAINED IN THE RRP, APIs OR SOFTWARE    #
rem # WILL MEET REGISTRAR'S REQUIREMENTS, OR THAT THE OPERATION OF THE RRP,    #
rem # APIs OR SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE,OR THAT DEFECTS IN  #
rem # THE RRP, APIs OR SOFTWARE WILL BE CORRECTED.  FURTHERMORE, NSI DOES NOT  #
rem # WARRANT NOR MAKE ANY REPRESENTATIONS REGARDING THE USE OR THE RESULTS OF #
rem # THE RRP, APIs, SOFTWARE OR RELATED DOCUMENTATION IN TERMS OF THEIR       #
rem # CORRECTNESS, ACCURACY, RELIABILITY, OR OTHERWISE.  SHOULD THE RRP, APIs  #
rem # OR SOFTWARE PROVE DEFECTIVE, REGISTRAR ASSUMES THE ENTIRE COST OF ALL    #
rem # NECESSARY SERVICING, REPAIR OR CORRECTION.                               #
rem ############################################################################


SET JAVAHOME=c:\jdk1.1.7a
SET CLASSPATH=..\..\..\..\..\..;%JAVAHOME%\lib\classes.zip

@ECHO JAVAHOME = %JAVAHOME%
@ECHO CLASSPATH = %CLASSPATH%
@ECHO Compiling Java code...
@%JAVAHOME%\bin\javac -O *.java
@ECHO Done!
