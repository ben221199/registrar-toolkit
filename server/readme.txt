NSI Registry
Copyright (c) 1999 Network Solutions, Inc. All rights reserved.

Network Solutions, Inc.
505 Huntmar Park Dr.
Herndon, VA 20170
USA
+1 703 925 6999
mailto:info@NSIRegistry.net

TABLE OF CONTENTS
1. Introduction
2. Server Description
3. Contact Information

1. Introduction

The rrps is an RRP server stub written in Perl for the purpose of 
testing RRP command functionality prior to entering the OT&E environment.
The current version of the server stub does not store any information 
sent to it. The server keeps track of sessions using the RRPCache.pm program. 
The reason for this is to allow for a true simulation of concurrent 
sessions. It was beyond the scope of this project to perform storage of 
information, but the RRPCache.pm can easily be extended to store domain 
names, name servers, etc. 

The code in the server stub is heavily commented. Please review the code
to get a thorough understanding of how it works.


2. Server Description

rrps
----
rrps is the main routine for the server stub. It starts up by reading
in the configuration file 'rrps.cfg'. It then performs a few initialization
tasks and opens a socket for accepting client connections. Once a client
connects the server stub forks off a child process to handle the incoming
client commands and goes back to accepting connections.

The child process goes into a read loop waiting for input from the client 
process. A select is performed on the client socket to allow for an idle 
timeout to occur. This is for compatibility with a real RRP server. Once 
a complete command has been sent, the child process will call a function
in the RRPCommands.pm program to attempt to process the command.

rrps.cfg
--------
This is the configuration file for the server stub. Please refer to 
the comments in the configuration file for more information about 
its use.

ipv4.cfg
--------
This configuration file identifies IP address block ranges reserved by
IANA.  IP addresses that fall within these ranges are NOT available
for general use, and they should thus NOT be used for name servers.

RRPCommands.pm
--------------
This program contains all of the functionality to check RRP command syntax,
and send RRP command responses. There is code to return random responses in
most of the command functions.

RRPCache.pm
-----------
This program contains the functionality to keep track of active sessions.
The main routine forks a child to manage session caching. Prior to 
starting the child process, the main routine initializes the cache.
The cache initialization opens a socket pair for allowing the cache 
child and the client child processes to communicate.

RRPUtils.pm
-----------
This program contains some miscellaneous functions.


3. Contact Information

Please use the NSI Registry's open source mailing list for questions
about the RRP Server stub. The following URL describes how to
subscribe to the mailing list: 

http://www.nsiregistry.com/opensrc/mailing.html

If you are in need of additional assistance, Registrar Support 
will be provided by the NSI Registry Help Desk. Support is available 
24 hours a day, 7 days a week.

Help Desk Phone: +1 703 925 6999
Help Desk Email: info@NSIRegistry.net
