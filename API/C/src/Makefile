############################################################################
# Copyright (C) 2000 VeriSign, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# VeriSign Global Registry Service
# 21345 Ridgetop Circle
# Dulles, VA 20166
############################################################################
# The RRP, APIs and Software are provided "as-is" and without any warranty
# of any kind.  NSI EXPRESSLY DISCLAIMS ALL WARRANTIES AND/OR CONDITIONS,
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# AND CONDITIONS OF MERCHANTABILITY OR SATISFACTORY QUALITY AND FITNESS FOR
# A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  NSI DOES
# NOT WARRANT THAT THE FUNCTIONS CONTAINED IN THE RRP, APIs OR SOFTWARE
# WILL MEET REGISTRAR'S REQUIREMENTS, OR THAT THE OPERATION OF THE RRP,
# APIs OR SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE,OR THAT DEFECTS IN
# THE RRP, APIs OR SOFTWARE WILL BE CORRECTED.  FURTHERMORE, NSI DOES NOT
# WARRANT NOR MAKE ANY REPRESENTATIONS REGARDING THE USE OR THE RESULTS OF
# THE RRP, APIs, SOFTWARE OR RELATED DOCUMENTATION IN TERMS OF THEIR
# CORRECTNESS, ACCURACY, RELIABILITY, OR OTHERWISE.  SHOULD THE RRP, APIs
# OR SOFTWARE PROVE DEFECTIVE, REGISTRAR ASSUMES THE ENTIRE COST OF ALL
# NECESSARY SERVICING, REPAIR OR CORRECTION.
############################################################################

#
# Compiler settings
#
CC = /usr/local/bin/gcc
INCLUDE = ../include
CFLAGS = -W -O -I$(INCLUDE) -fPIC

#
# Archive settings
#
AR = /usr/ccs/bin/ar
ARFLAGS = cru

#
# LD_LIBRARY_PATH
#
LD_LIBRARY_PATH=.:/usr/lib:/usr/local/lib
export LD_LIBRARY_PATH


#
# Directory to build library file in
#
LIBDIR = ../lib

DEPENDFLAGS = -E -MM
LIBS = -lrrpconnection -lrrpapi -lnsl -lsocket
LDFLAGS = -L$(LIBDIR) $(LIBS)

PRODUCTS = \
	$(LIBDIR)/librrpconnection.a \
	$(LIBDIR)/librrpapi.a \
	rrpAPIExample

OBJECTS = \
	rrpAPI.o \
	rrpInternalError.o \
	rrpVector.o \
	rrpProperties.o


all: env_check Makefile.dependencies $(PRODUCTS)

env_check:
	@(if [ ! -d "$(LIBDIR)" ]; then \
		echo ; echo "########################"; \
		echo "BUILD ENVIRONMENT ERROR!"; \
		echo "########################"; \
		echo ; echo "lib directory $(LIBDIR) does not exist. Please verify" ; \
		echo "that the 'LIBDIR' variable is set properly." ; \
		echo ; echo ; exit 1; \
	fi)
	@(if [ ! -x "$(CC)" ]; then \
		echo ; echo "########################"; \
		echo "BUILD ENVIRONMENT ERROR!"; \
		echo "########################"; \
		echo ; echo "Cannot locate executable compiler."; \
		echo "Please make sure that the 'CC' variable is"; \
		echo "set proprely before running make."; \
		echo ; echo ; exit 1; \
	fi)


clean:
	rm -f $(LIBDIR)/*.a *.o $(LIBDIR)/*.so  $(PRODUCTS) $(OBJECTS)

Makefile.dependencies depend:
	$(CC) -I$(INCLUDE) $(DEPENDFLAGS) *.c > Makefile.dependencies

$(LIBDIR)/librrpconnection.a: rrpConnection.o
	$(AR) $(ARFLAGS) $@ $^

$(LIBDIR)/librrpapi.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(LIBDIR)/librrpconnection.so : rrpConnection.o
	${CC} -o $@ -shared rrpConnection.o

$(LIBDIR)/librrpapi.so : $(OBJECTS)
	${CC} -o $@ -shared $(OBJECTS)

%: %.o
	$(CC) -o $@ $< $(LDFLAGS)


-include Makefile.dependencies

