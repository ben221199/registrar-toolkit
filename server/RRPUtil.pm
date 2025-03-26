# ===========================================================================
# Copyright (C) 2000 Network Solutions, Inc.
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
# Network Solutions, Inc. Registry
# 505 Huntmar Park Dr.
# Herndon, VA 20170
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
# File name: RRPUtil.pm
# Author: Scott Hollenbeck <shollenb@netsol.com>
# 
# =========================================================================
package RRPUtil;
use strict;

use Exporter;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

$VERSION = 1.10;
@ISA = qw(Exporter);

@EXPORT = qw(expiration_date inet_to64 renew_check 
             validate_ipv4string);

@EXPORT_OK = qw(expiration_date inet_to64 renew_check 
                validate_ipv4string);

%EXPORT_TAGS = (
  Functions => [qw(expiration_date inet_to64 renew_check
                   validate_ipv4string)]);

# =========================================================================
# This procedure builds an expiration date string based on the current date
# and a provided number of years for the registration period.  Leap years
# are accounted for manually to avoid dependencies on third-party libraries.
# =========================================================================
sub expiration_date {
  my ($start_year, $period) = @_;

  my $exp_date = "";
  my $now = time();
  my $end_year = my $leap_years = my $temp_year = my $then = my $years = 0;
  my $sec = my $min = my $hour = my $mday = my $mon = my $year = 0;
  my $seconds_per_day = 60 * 60 * 24;
  my $seconds_per_year = 60 * 60 * 24 * 365;

  # Calculate the expiration date based on the period and the current date.
  # The year offset is calculated in seconds for addition to the epoch offset.
  $years = $seconds_per_year * $period;

  # Add the offset to the current time and calculate the date.
  $then = $now + $years;

  # Account for leap years.  Start by getting the current date.  We need this
  # to determine where we are in the current year and how many leap years are
  # ahead of us.
  ($mday, $mon, $year) = (localtime)[3,4,5];
  ++$mon;
  $year += 1900;

  # Add time if a non-default starting year was given.
  if ($start_year != 0) {
    my $year_delta = $start_year - $year;

    if ($year_delta > 0) {
      $then += $year_delta * $seconds_per_year;
      $end_year = $year + $year_delta + $period;
    }
  }
  else {
    $end_year = $year + $period;
  }

  # OK, we know the current year.  Calculate the number of leap years between
  # now and the end of the registration period.  So what makes a leap year?
  # Years divisible by four are leap years, unless...
  # Years also divisible by 100 are not leap years, except...
  # Years divisible by 400 are leap years.

  $end_year = $year + ($start_year - $year) + $period;
  foreach $temp_year ($year .. $end_year) {
    if (($temp_year % 4) == 0) {
      if (($temp_year % 100) == 0) {
        if (($temp_year % 400) == 0) {
          if ($temp_year == $year) {
            # The current year is a leap year.  Don't note the year unless 
            # we haven't yet passed the leap date.
            if (($mon == 1) || ($mon == 2 && $mday < 29)) {
              ++$leap_years;
            }
          }
          else {
            ++$leap_years;
          }
        }
      }
      else {
        if ($temp_year == $year) {
          # The current year is a leap year.  Don't note the year unless we 
          # haven't yet passed the leap date.
          if (($mon == 1) || ($mon == 2 && $mday < 29)) {
            ++$leap_years;
          }
        }
        else {
          ++$leap_years;
        }
      }
    }
  }
  $then += $seconds_per_day * $leap_years;

  # Now convert from epoch to string format.
  ($sec, $min, $hour, $mday, $mon, $year, undef, undef, undef)
   = localtime($then);
  ++$mon; # Account for starting at zero.
  $year += 1900; # Account for epoch begin.
  $mon = "0" . $mon unless (length($mon) == 2);
  $hour = "0" . $hour unless (length($hour) == 2);
  $min = "0" . $min unless (length($min) == 2);
  $sec = "0" . $sec unless (length($sec) == 2);
  $exp_date = $year . "-" . $mon . "-" . $mday . " "
   . $hour . ":" . $min . ":" . $sec . ".0";

  # Finished!
  return $exp_date;
}

# =========================================================================
# This procedure takes a string representation of an IP address and converts
# it into a 64-bit integer value.  It takes each octet from left to right,
# doing shifts of 8 bits per octet to create the return value.  Keep in mind
# that a left shift of 8 bits is essentially the same as multiplying by 256.
# =========================================================================
sub inet_to64 {
  my ($stringIP) = @_;

  my @elem = ();
  my $octet = "";
  my $packedIP = 0;
  my $response = -1;
  my $temp;

  # Make sure the address is syntactically valid.
  if (validate_ipv4string($stringIP) != 0) {
    return $response;
  }

  # Address checks out, so parse it and grab each octet.
  @elem = split(/\./, $stringIP);

  # Multiply each octet into the return value.
  # Shift the first octet three times.
  $temp = $elem[0] * 256 * 256 * 256;
  $packedIP += $temp;

  # Shift the second octet twice.
  $temp = $elem[1] * 256 * 256;
  $packedIP += $temp;

  # Shift the third octet once.
  $temp = $elem[2] * 256;
  $packedIP += $temp;

  # Use the fourth octet as-is.
  $packedIP += $elem[3];

  # Finished!  Return the result.
  return ($packedIP);
}

# =========================================================================
# This procedure determines if there's time available to complete a RENEW
# within the system-defined maximum registration period.
# =========================================================================
sub renew_check {
  my ($exp_year, $period, $max_period) = @_;

  my $year_now = 0;

  # Determine the current year.
  $year_now = (localtime)[5];
  $year_now += 1900;

  # Calulate the amount of time available and note an error if there's not
  # enough room to add the requested period.
  if (($exp_year - $year_now) + $period > $max_period) {
    return 1;
  }
  else {
    return 0;
  }
}

# =========================================================================
# This procedure validates an IPv4 address.
# =========================================================================
sub validate_ipv4string {
  my ($ipv4string) = @_;

  my @elem = ();
  my $octet = "";
  my $response = 0;

  # Split the address into octets, check the octets, and return results.
  if ($ipv4string =~ /\./) {
    @elem = split(/\./, $ipv4string);

    # Make sure there are 4 octets.
    if (@elem != 4) {
      $response = 1;
      return $response;
    }

    # Look at each octet.
    foreach $octet (@elem) {
      if ($octet < 0 || $octet > 255) {
        $response = 1;
        last;
      }
    }
  }
  else {
    # Didn't find a ".", so the address is invalid.
    $response = 1;
  }

  # Return results.
  return $response;
}

# Last line.
1;
