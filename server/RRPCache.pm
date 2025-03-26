# ===========================================================================
# Copyright (C) 2000 VeriSign, Inc.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
# 
# VeriSign Global Registry Service
# 21345 Ridgetop Circle
# Dulles, VA 20166
# =========================================================================
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
# =========================================================================
#
# RRPCache: A Simple NSI Registry Registrar protocol server cache to store
#           session information and possibly command statistics.
#           This mechanism can be expanded to add storing of domain and
#           name server information. The current intent of this cache is
#           to maintain concurrent sessions to limit the number of sessions
#           to that set in the configuration file 'rrps.cfg'.
#
# File name: RRPCache.pm
# Author: Chris Bason <chrisb@netsol.com>
# 
# =========================================================================
package RRPCache;
use IO::File;
use POSIX ();
use strict;
use Socket;

use Exporter;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

$VERSION = 1.10;
@ISA = qw(Exporter);

@EXPORT = qw(cache_init process_cache_requests increment_session_count 
             decrement_session_count);

@EXPORT_OK = qw(cache_init process_cache_requests increment_session_count
                decrement_session_count);

%EXPORT_TAGS = (
  Functions => [qw(cache_init process_cache_requests increment_session_count
                   decrement_session_count)]);

# Private global variables
my $session_count = 0;
my $cfg;
my %domains;
my %nameservers;

# =========================================================================
#
# Initialize the RRP server cache.
#
# =========================================================================
sub cache_init {

  # set config hash here
  ($cfg) = @_;

  # OK, now we are going to create a socket for use by all
  # forked children. Each child will lock the CACHEOUT when it needs to use
  # the CACHEOUT socket using flock() before performing an operation.
  socketpair(CACHEOUT, CACHEIN, AF_UNIX, SOCK_STREAM, 0)
   or die "RRPS (Fatal Error): Unable to create socket for cache lock!";

  # open the lock file.
  my $lockfile = POSIX::tmpnam();
  open(LOCK_FH, "> $lockfile")
   or die "RRPS (Fatal Error): Unable to create lockfile: $!";
}

# =========================================================================
#
# Process cache requests from child processes.
#
# =========================================================================
sub process_cache_requests {
  my $inbuf;

  for (; ;) {
    $inbuf = <CACHEIN>;

    if (defined $inbuf) {
      chomp($inbuf);

      if ($inbuf eq "increment session_count") {
        if ($session_count < $cfg->{"MaxSessions"}) {
          ++$session_count;
          print CACHEIN "success\n";
        }
        else {
          print CACHEIN "failure\n";
        }
      }
      elsif ($inbuf eq "decrement session_count") {
        print CACHEIN "success\n";
        --$session_count;
      }

      flush CACHEIN;
    }
  }
}

# =========================================================================
#
# Function to check then increment the cached session count.
#
# =========================================================================
sub increment_session_count {
  my $retc;
  my $inbuf;

  # lock the socket
  lock_cache();

  # Check the session count here; we have lock so it is ok to touch
  # variable
  print CACHEOUT "increment session_count\n";

  flush CACHEOUT;

  $inbuf = <CACHEOUT>;

  if (defined $inbuf) {
    chomp($inbuf);

    if ($inbuf eq "success") {
      $retc = 1;
    }
    else {
      $retc = 0;
    }
  }
  else {
    $retc = 0;
  }
  
  # unlock socket 
  unlock_cache();

  return $retc;
}

# =========================================================================
#
# Function to decrement the cached session count.
#
# =========================================================================
sub decrement_session_count {
  my $new;
  my $inbuf;

  # lock the socket
  lock_cache();

  print CACHEOUT "decrement session_count\n";

  flush CACHEOUT;

  $inbuf = <CACHEOUT>;

  if (defined $inbuf) {
    # nothing to do
  }

  # unlock socket
  unlock_cache();
}

sub lock_cache {
  flock LOCK_FH, 2 or die "RRPS (Fatal Error): Unable to lock: $!";
}

sub unlock_cache {
  flock LOCK_FH, 8 or die "RRPS (Fatal Error): Failed to release lock: $!";
}

# Last line.
1;
