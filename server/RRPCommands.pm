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
# File name: RRPCommands.pm
# Author: Scott Hollenbeck <shollenb@netsol.com>
# Modified by: Chris Bason <chrisb@netsol.com>
# 
# =========================================================================
package RRPCommands;
use strict;

use Exporter;
use RRPCache;
use RRPUtil;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

$VERSION = 1.10;
@ISA = qw(Exporter);

@EXPORT = qw(process_command add_command check_command del_command 
             describe_command mod_command quit_command renew_command
             session_command status_command transfer_command 
             invalid_command get_response);

@EXPORT_OK = qw(process_command add_command check_command del_command 
                describe_command mod_command quit_command renew_command
                session_command status_command transfer_command 
                invalid_command get_response);

%EXPORT_TAGS = (
  Functions => [qw(process_command add_command check_command del_command
                   describe_command mod_command quit_command renew_command
                   session_command status_command transfer_command 
                   invalid_command get_response)]);

# Private global variables.
my $crlf = "\r\n";
my @ipv4_table = ();
my @ipv6_table = ();
my $session_established = 0;
my $session_try_count = 0;
my $version = "1.1.0";

# Returned attribute strings.
my $attr_created_by = "created by:";
my $attr_created_date = "created date:";
my $attr_exp_date = "registration expiration date:";
my $attr_ipaddress = "ipaddress:";
my $attr_nameserver = "nameserver:";
my $attr_registrar = "registrar:";
my $attr_updated_by = "updated by:";
my $attr_updated_date = "updated date:";
my $attr_status1 = "status:ACTIVE";
my $attr_status2 = "status:REGISTRY-HOLD";
my $attr_status3 = "status:REGISTRY-LOCK";
my $attr_status4 = "status:REGISTRAR-HOLD";
my $attr_status5 = "status:REGISTRAR-LOCK";
my $attr_status6 = "status:REGISTRY-DELETE-NOTIFY";
my $attr_transfer_date = "registrar transfer date:";
my $is_cctldns = 0;

# Create a hash table for RRP command processing procedures.
# Note the lower case keys.
my %rrp_commands = (
  'add'      => \&add_command,
  'check'    => \&check_command,
  'del'      => \&del_command,
  'describe' => \&describe_command,
  'mod'      => \&mod_command,
  'quit'     => \&quit_command,
  'renew'    => \&renew_command,
  'session'  => \&session_command,
  'status'   => \&status_command,
  'transfer' => \&transfer_command,
  'invalid'  => \&invalid_command);

# This is the hash table for protocol response codes.
my %response_codes = (
  '200' => " Command completed successfully",
  '210' => " Domain name available",
  '211' => " Domain name not available",
  '212' => " Name server available",
  '213' => " Name server not available",
  '220' => " Command completed successfully. Server closing connection",
  '420' => " Command failed due to server error. Server closing connection",
  '421' => " Command failed due to server error. Client should try again",
  '500' => " Invalid command name",
  '501' => " Invalid command option",
  '502' => " Invalid entity value",
  '503' => " Invalid attribute name",
  '504' => " Missing required attribute",
  '505' => " Invalid attribute value syntax",
  '506' => " Invalid option value",
  '507' => " Invalid command format",
  '508' => " Missing required entity",
  '509' => " Missing option",
  '520' => " Server closing connection. Client should try opening new connection",
  '521' => " Too many sessions. Server closing connection",
  '530' => " Authentication failed",
  '531' => " Authorization failed",
  '532' => " Domain names linked with name server",
  '533' => " Domain name has active name servers",
  '534' => " Domain name has not been flagged for transfer",
  '535' => " Restricted IP address",
  '536' => " Domain already flagged for transfer",
  '540' => " Attribute value not unique",
  '541' => " Invalid attribute value",
  '542' => " Invalid old value for an attribute",
  '543' => " Final or implicit attribute cannot be updated",
  '544' => " Entity on hold",
  '545' => " Entity reference not found",
  '546' => " Credit limit exceeded",
  '547' => " Invalid command sequence",
  '548' => " Domain not up for renewal",
  '549' => " Command failed",
  '550' => " Parent domain not registered",
  '551' => " Parent domain status does not allow for operation",
  '552' => " Domain status does not allow for operation",
  '553' => " Operation not allowed. Domain pending transfer",
  '554' => " Domain already registered",
  '555' => " Domain already renewed",
  '556' => " Maximum registration period exceeded");

# These hash tables are for command parsing.  Each of the keys identifies a
# protocol token and a rough approximation of the valid values that are
# associated with the token.

my %add_elements = (
  'attributes' => undef,
  'options'    => undef);

my %add_table = (
  'entityname' => \&validate_entity,
  'domainname' => \&validate_domain,
  'nameserver' => \&validate_server,
  '-period'    => \&validate_period,
  'ipaddress'  => \&validate_ipv4address);

my $min_add_servers = 0;
my $max_add_servers = 13;
my $min_add_addresses = 1;
my $max_add_addresses = 13;

my %check_elements = (
  'attributes'    => undef);

my %check_table = (
  'entityname' => \&validate_entity,
  'domainname' => \&validate_domain,
  'nameserver' => \&validate_server);

my %del_elements = (
  'attributes'    => undef);

my %del_table = (
  'entityname' => \&validate_entity,
  'domainname' => \&validate_domain,
  'nameserver' => \&validate_server);

my %describe_elements = (
  'options'    => undef);

my %describe_table = (
  '-target' => \&validate_target);

my %mod_elements = (
  'attributes'    => undef,
  'flag'          => "=");

my %mod_table = (
  'entityname'    => \&validate_entity,
  'domainname'    => \&validate_domain,
  'nameserver'    => \&validate_server,
  'newnameserver' => \&validate_server,
  'ipaddress'     => \&validate_ipv4address);

my %quit_elements = ();

my %quit_table = ();

my %renew_elements = (
  'attributes' => undef,
  'options'    => undef);

my %renew_table = (
  'entityname'             => \&validate_entity,
  'domainname'             => \&validate_domain,
  '-period'                => \&validate_period,
  '-currentexpirationyear' => \&validate_year);

my %session_elements = (
  'options'    => undef);

my %session_table = (
  '-id'          => \&validate_word,
  '-password'    => \&validate_password,
  '-newpassword' => \&validate_password);

my %status_elements = (
  'attributes'    => undef);

my %status_table = (
  'entityname' => \&validate_entity,
  'domainname' => \&validate_domain,
  'nameserver' => \&validate_server);

my %transfer_elements = (
  'attributes' => undef,
  'options'    => undef);

my %transfer_table = (
  'entityname'  => \&validate_entity,
  'domainname'  => \&validate_domain,
  '-approve'    => \&validate_yesno);

# These hash tables are used to create weighted random numbers to simulate
# error responses when command syntax is OK.  The key corresponds to an RRP
# response code, and the value corresponds to a weight of probable return.
# The weights can be changed freely to make certain responses appear more or
# less frequently.

my %check_domain_responses = (
  '210' => .97,
  '420' => .01,
  '421' => .01,
  '549' => .01);

my %check_server_responses = (
  '212' => .97,
  '420' => .01,
  '421' => .01,
  '549' => .01);

my %check_responses = (
  'domain'     => {%check_domain_responses},
  'nameserver' => {%check_server_responses});

my %response_hash = (
  '200' => .97,
  '420' => .01,
  '421' => .01,
  '549' => .01);

# =========================================================================
#
# Function to process an RRP command from the client.
#
# =========================================================================
sub process_command {
  my ($cfg, $rrp_command) = @_;
  my @lines = ();
  my $command = "";
  my $local_command = "";
  my $response = "";
  my $stay_open = -1;

  # Convert all command characters to lower case.
  $local_command = lc($rrp_command);

  # Split the command into an array of individual lines.
  @lines = split(/$crlf/, $local_command);

  # Call the appropriate command processing subroutine.
  # Note that invalid commands will fall through the "if" statement.
  $command = $lines[0];

  # Note the IPv4 address table.
  @ipv4_table = \@main::ipv4;

  # Note the IPv6 address table.
  @ipv6_table = \@main::ipv6;

  if ($rrp_commands{$command}) {
    ($stay_open, $response) =
        $rrp_commands{$command}->(\@lines, $cfg);
  }
  else {
    ($stay_open, $response) =
        $rrp_commands{"invalid"}->(\@lines, $cfg);
  }

  # Return the response after adding terminators.
  $response .= $crlf . "." . $crlf;
  return ($stay_open, $response);
} 

# =========================================================================
#
# RRP Add command
#
# =========================================================================
sub add_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the ADD command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%add_table, \%add_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an entity name.
    if (!($seen{"entityname"})) {
      $response_key = "508";
    }

    # Must also have either a domain name or name server name.
    elsif (!$seen{"domainname"} && !$seen{"nameserver"}) {
      $response_key = "504";
    }

    # If a domain name is being added, there can also be name 
    # servers (but no "ipaddress" tokens).
    elsif ($seen{"domainname"}) {
      if ($seen{"ipaddress"}) {
        $response_key = "503";
      }

      # Check name server count.
      elsif ($seen{"nameserver"}) {
        # Was the maximum exceeded?
        if ($seen{"nameserver"} > $max_add_servers) {
          $response_key = "507";
        }
        # Was the minimum met?  The min may be zero, but this check is here to
        # keep the logic complete.
        elsif ($seen{"nameserver"} < $min_add_servers) {
          $response_key = "504";
        }
      }

      # Check for inputs to cause forced error conditions.
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
        $response_key = "540";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError2"})) {
        $response_key = "554";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError4"})) {
        $response_key = "546";
      }
    }

    # If a name server is being added, there MUST also be IP 
    # addresses (but no "domainname" tokens).
    else {
      if ($seen{"domainname"}) {
        $response_key = "503";
      }
      # Check address count.
      elsif ($seen{"ipaddress"}) {
        # Was the maximum exceeded?
        if ($seen{"ipaddress"} > $max_add_addresses) {
          $response_key = "507";
        }
        # Was the minimum met?  The min may be zero, but this check is 
        # here to keep the logic complete.
        elsif ($seen{"ipaddress"} < $min_add_addresses) {
          if($is_cctldns == 0) {
            $response_key = "504";
          }
        }
        elsif ($seen{"ipaddress"} >= $min_add_addresses) {
          if($is_cctldns == 1) {
            $response_key = "503";
          }
        }

        # Check for inputs to cause forced error conditions.
        elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError1"})) {
          $response_key = "540";
        }
        elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError2"})) {
          $response_key = "550";
        }
        elsif ($tokens{"ipaddress"} eq $cfg->{"NSAddress"}) {
          $response_key = "540";
        }
      }
      else {
        # No IP addresses specified.  At least one is required.
          if($is_cctldns == 0) {
            $response_key = "504";
          }
      }
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    $response_key = rand_response(%response_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  # Add status info if needed.
  if ($response_key eq "200" && $seen{"domainname"}) {
    my $exp_date = my $period = "";

    # Calculate the expiration date based on the period and the current date.
    if ($seen{"-period"}) {
      $period = $tokens{"-period"};
    }
    else {
      $period = $cfg->{"DefaultAddPeriod"};
    }
    $exp_date = expiration_date(0, $period);

    # Finish building the adjusted response.
    $response .= $crlf . $attr_status1 . $crlf . $attr_exp_date . $exp_date;
  }

  return ($status, $response);
}

# =========================================================================
#
# RRP Check command
#
# =========================================================================
sub check_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the CHECK command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%check_table, \%check_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an entity name.
    if (!($seen{"entityname"})) {
      $response_key = "508";
    }

    # Must also have either a domain name or name server name, but not both.
    elsif ($seen{"domainname"} && $seen{"nameserver"}) {
      $response_key = "503";
    }
    elsif (!$seen{"domainname"} && !$seen{"nameserver"}) {
      $response_key = "504";
    }

    # Check for inputs to cause forced error conditions.
    if ($seen{"domainname"}) {
      if ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
        $response_key = "211";
      }
    }
    if ($seen{"nameserver"}) {
      if ($tokens{"nameserver"} eq lc($cfg->{"ServerError1"})) {
        $response_key = "213";
      }
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    my $resp_hash = $check_responses{$tokens{"entityname"}};

    $response_key = rand_response(%$resp_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  # Add an IPAddress line if needed.
  if ($response_key eq "213") {
    $response .= $crlf . "ipAddress:" . $cfg->{"NSAddress"};
  }

  return ($status, $response);
}

# =========================================================================
#
# RRP Delete command
#
# =========================================================================
sub del_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the DEL command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%del_table, \%del_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an entity name.
    if (!($seen{"entityname"})) {
      $response_key = "508";
    }

    # Must also have either a domain name or name server name, but not both.
    elsif ($seen{"domainname"} && $seen{"nameserver"}) {
      $response_key = "503";
    }
    elsif (!$seen{"domainname"} && !$seen{"nameserver"}) {
      $response_key = "504";
    }

    # Check for known forced error names.
    if ($seen{"domainname"}) {
      if ($tokens{"domainname"} eq lc($cfg->{"DomainError4"})) {
        $response_key = "533";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistryHold"})) {
        $response_key = "544";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistryDelNotify"})) {
        $response_key = "533";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistrarHold"})) {
        $response_key = "544";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError3"})) {
        $response_key = "545";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
        $response_key = "552";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError2"})) {
        $response_key = "553";
      }
    }
    else {
      if ($tokens{"nameserver"} eq lc($cfg->{"ServerError4"})) {
        $response_key = "532";
      }
      elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError3"})) {
        $response_key = "545";
      }
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    $response_key = rand_response(%response_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  return ($status, $response);
}

# =========================================================================
#
# RRP Describe command
#
# =========================================================================
sub describe_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the DESCRIBE command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%describe_table, \%describe_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # No semantic checks required.  The protocol version info is returned even if
  # no options are provided.

  # Return results.
  $response = $response_key . $response_codes{$response_key};

  # Add description info to the returned results.
  if ($response_key eq "200") {
    $response .= $crlf . "Protocol:RRP " . $version;
  }

  return ($status, $response);
}

# =========================================================================
#
# RRP Modify command
#
# =========================================================================
sub mod_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the MOD command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%mod_table, \%mod_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an entity name.
    if (!($seen{"entityname"})) {
      $response_key = "508";
    }

    # Must also have either a domain name or name server name.
    elsif (!$seen{"domainname"} && !$seen{"nameserver"}) {
      $response_key = "504";
    }

    # If a domain name is being modified, there can also be name servers 
    # (but no "ipaddress" tokens).
    elsif ($seen{"domainname"}) {
      if ($seen{"ipaddress"}) {
        $response_key = "503";
      }

      # Check name server count.
      elsif ($seen{"nameserver"}) {
        # Was the maximum exceeded?
        if ($seen{"nameserver"} > $max_add_servers) {
          $response_key = "507";
        }
        # Was the minimum met?  The min may be zero, but this check is here to
        # keep the logic complete.
        elsif ($seen{"nameserver"} < $min_add_servers) {
          $response_key = "504";
        }
      }

      # Check for inputs to cause forced error conditions.
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
        $response_key = "540";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError3"})) {
        $response_key = "545";
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError5"})) {
        $response_key = "531";
      }
    }

    # If a name server is being added, there MUST also be IP addresses (but no
    # "domainname" tokens).
    else {
      if ($seen{"domainname"}) {
        $response_key = "503";
      }
      # Check address count.
      elsif ($seen{"ipaddress"}) {
        # Was the maximum exceeded?
        if ($seen{"ipaddress"} > $max_add_addresses) {
          $response_key = "507";
        }
        # Was the minimum met?  The min may be zero, but this check is here to
        # keep the logic complete.
        elsif ($seen{"ipaddress"} < $min_add_addresses) {
          if($is_cctldns == 0) {
            $response_key = "504";
          }
        }
        elsif ($seen{"ipaddress"} >= $min_add_addresses) {
          if($is_cctldns == 1) {
            $response_key = "503";
          }
        }

        # Check for inputs to cause forced error conditions.
        elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError1"})) {
          $response_key = "540";
        }
        elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError2"})) {
          $response_key = "551";
        }
        elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError3"})) {
          $response_key = "545";
        }
        elsif ($tokens{"nameserver"} eq lc($cfg->{"ServerError5"})) {
          $response_key = "531";
        }
        elsif ($tokens{"ipaddress"} eq $cfg->{"NSAddress"}) {
          $response_key = "540";
        }
      }
      else {
        # No IP addresses specified.  At least one is required.
          if($is_cctldns == 0) {
            $response_key = "504";
          }
      }
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    $response_key = rand_response(%response_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  return ($status, $response);
}

# =========================================================================
#
# RRP Quit command
#
# =========================================================================
sub quit_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my $seen_p;
  my $status = 1;
  my $tokens_p;

  # Check command structure.
  # There are currently no command parameters, so the returned hashes should
  # be empty.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%quit_table, \%quit_elements);

  # Semantic checks.
  if ($response_key eq "200") {
    $response_key = "220";
    $status = 0;
  }

  # decrement the session count here
  decrement_session_count();

  # Results.  No weighted random response is returned.
  $response = $response_key . $response_codes{$response_key};

  return ($status, $response);
}

# =========================================================================
#
# RRP Renew command
#
# =========================================================================
sub renew_command {
  my ($lines, $cfg) = @_;

  my $exp_year = my $period = 0;
  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the RENEW command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%renew_table, \%renew_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an entity name.
    if (!($seen{"entityname"})) {
      $response_key = "508";
    }

    # Must also have a domain name.
    elsif (!$seen{"domainname"}) {
      $response_key = "504";
    }

    # If a period was provided, the current expiration year must 
    # also be provided.
    elsif ($seen{"-period"}) {
      if (!$seen{"-currentexpirationyear"}) {
        $response_key = "509";
      }
    }

    # If a current expiration year was provided, the period must 
    # also be provided.
    elsif ($seen{"-currentexpirationyear"}) {
      if (!$seen{"-period"}) {
        $response_key = "509";
      }
    }

    # Process a syntactically-correct renewal.
    # Check for inputs to cause forced error conditions.
    if ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
      $response_key = "548";
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError2"})) {
      $response_key = "555";
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError3"})) {
      $response_key = "545";
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError4"})) {
      $response_key = "546";
    }

    # There is a limit on the number of years available for a RENEW.  Make sure
    # there's enough room.
    else {
      my $max_period = $cfg->{"MaxRegistrationPeriod"};

      if ($seen{"-currentexpirationyear"}) {
        $exp_year = $tokens{"-currentexpirationyear"};
      }
      else {
        $exp_year = $cfg->{"ExpirationYear"};
      }
      if ($seen{"-period"}) {
        $period = $tokens{"-period"};
      }
      else {
        $period = $cfg->{"DefaultRenewPeriod"};
      }

      # Is there enough room?  This check doesn't get any more granular 
      # than years, though the "real" implementation works at a much 
      # finer level.
      if (renew_check($exp_year, $period, $max_period) != 0) {
        $response_key = "556";
      }
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    $response_key = rand_response(%response_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  # Add status info if needed.
  if ($response_key eq "200" && $seen{"domainname"}) {
    my $dom_status = $attr_status1;
    # Calculate the expiration date based on the period and the current date.
    my $exp_date = expiration_date($exp_year, $period);

    # Finish building the adjusted response.
    if ($tokens{"domainname"} eq lc($cfg->{"RegistryHold"})) {
      $dom_status = $attr_status2;
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"RegistryLock"})) {
      $dom_status = $attr_status3;
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"RegistryDelNotify"})) {
      $dom_status = $attr_status4;
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"RegistrarHold"})) {
      $dom_status = $attr_status5;
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"RegistrarLock"})) {
      $dom_status = $attr_status6;
    }
    $response .= $crlf . $dom_status . $crlf . $attr_exp_date . $exp_date;
  }

  return ($status, $response);
}

# =========================================================================
#
# RRP Session command
#
# =========================================================================
sub session_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # Make sure there isn't already an active session.
  if ($session_established == 1) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%session_table, \%session_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an ID and password.
    if (!defined %seen || $seen{"-id"} == 0 || $seen{"-password"} == 0) {
      $response_key = "509";
    }

    # Check the provided values against the configuration values.
    elsif ($tokens{"-id"} ne $cfg->{"RegistrarID"} ||
           $tokens{"-password"} ne $cfg->{"RegistrarPW"}) {
      $response_key = "530";
    }

    # Check for a password change request.
    elsif ($seen{"-newpassword"}) {
      $cfg->{"RegistrarPW"} = $tokens{"-newpassword"};
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    if (($response_key = rand_response(%response_hash)) eq "200") {

      # Don't create more than the maximum number of sessions.
      if (increment_session_count()) {
        $session_established = 1;
      }
      else {
        $status = 0;
        $response_key = "521";
      }
    }
    # Did we get an error that requires closing the connection?
    elsif ($response_key eq "420") {
      $status = 0;
    }
  }

  # Note failed session attempts and disconnect after a configurable maximum 
  # has been met.
  if ($status == 1 && $response_key ne "200") {
    if ($session_try_count >= $cfg->{"MaxSessionAttempts"}) {
      $status = 0;
      $response_key = "520";
    }
    else {
      ++$session_try_count;
    }
  }
  $response = $response_key . $response_codes{$response_key};

  return ($status, $response);
}

# =========================================================================
#
# RRP Status command
#
# =========================================================================
sub status_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the STATUS command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%status_table, \%status_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have an entity name.
    if (!($seen{"entityname"})) {
      $response_key = "508";
    }

    # Must also have either a domain name or name server name, but not both.
    elsif ($seen{"domainname"} && $seen{"nameserver"}) {
      $response_key = "503";
    }
    elsif (!$seen{"domainname"} && !$seen{"nameserver"}) {
      $response_key = "504";
    }

    # Check for known "unauthorized" names.
    if ($seen{"domainname"}) {
      if ($tokens{"domainname"} eq lc($cfg->{"DomainError5"})) {
        $response_key = "531";
      }
    }
    else {
      if ($tokens{"nameserver"} eq lc($cfg->{"ServerError5"})) {
        $response_key = "531";
      }
    }

    # Check for known "not found" names.
    if ($seen{"domainname"}) {
      if ($tokens{"domainname"} eq lc($cfg->{"DomainError3"})) {
        $response_key = "545";
      }
    }
    else {
      if ($tokens{"nameserver"} eq lc($cfg->{"ServerError3"})) {
        $response_key = "545";
      }
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    $response_key = rand_response(%response_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  # Add status information if a positive response is being returned.
  if ($response_key eq "200") {
    $response .= $crlf;

    if ($seen{"domainname"}) {
      my $dom_status = $attr_status1;
      my $server = "";
      my @servers = ();

      # Process domain name status.
      $response .= $attr_registrar . $cfg->{"RegistrarID"} . $crlf;
      if ($cfg->{"UpdateBy"}) {
        $response .= $attr_updated_by . $cfg->{"UpdateBy"} . $crlf;
      }
      else {
        $response .= $attr_updated_by . $cfg->{"RegistrarID"} . $crlf;
      }
      if ($tokens{"domainname"} eq lc($cfg->{"RegistryHold"})) {
        $dom_status = $attr_status2;
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistryLock"})) {
        $dom_status = $attr_status3;
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistryDelNotify"})) {
        $dom_status = $attr_status4;
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistrarHold"})) {
        $dom_status = $attr_status5;
      }
      elsif ($tokens{"domainname"} eq lc($cfg->{"RegistrarLock"})) {
        $dom_status = $attr_status6;
      }
      $response .= $dom_status . $crlf;
      if ($cfg->{"TransferDate"}) {
        $response .= $attr_transfer_date . $cfg->{"TransferDate"} . $crlf;
      }
      $response .= $attr_updated_date . $cfg->{"UpdateDate"} . $crlf;
      $response .= $attr_created_date . $cfg->{"CreateDate"} . $crlf;
      $response .= $attr_exp_date . $cfg->{"ExpireDate"} . $crlf;
      if ($cfg->{"CreateBy"}) {
        $response .= $attr_updated_by . $cfg->{"CreateBy"};
      }
      else {
        $response .= $attr_updated_by . $cfg->{"RegistrarID"};
      }

      # Process name server list.
      @servers = split(/,/, $cfg->{"NameServers"});
      foreach $server (@servers) {
        $response .= $crlf . $attr_nameserver . $server;
      }
    }
    else {
      my $address = "";
      my @addresses = ();

      # Process name server status.
      $response .= $attr_registrar . $cfg->{"RegistrarID"} . $crlf;
      if ($cfg->{"UpdateBy"}) {
        $response .= $attr_updated_by . $cfg->{"UpdateBy"} . $crlf;
      }
      else {
        $response .= $attr_updated_by . $cfg->{"RegistrarID"} . $crlf;
      }
      if ($cfg->{"TransferDate"}) {
        $response .= $attr_transfer_date . $cfg->{"TransferDate"} . $crlf;
      }
      $response .= $attr_updated_date . $cfg->{"UpdateDate"} . $crlf;
      $response .= $attr_created_date . $cfg->{"CreateDate"} . $crlf;
      if ($cfg->{"CreateBy"}) {
        $response .= $attr_updated_by . $cfg->{"CreateBy"};
      }
      else {
        $response .= $attr_updated_by . $cfg->{"RegistrarID"};
      }

      # Process IP address list.
      @addresses = split(/,/, $cfg->{"IPAddresses"});
      foreach $address (@addresses) {
        $response .= $crlf . $attr_ipaddress . $address;
      }
    }
  }

  return ($status, $response);
}

# =========================================================================
#
# RRP Transfer command
#
# =========================================================================
sub transfer_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "";
  my %seen = ();
  my $seen_p;
  my $status = 1;
  my %tokens = ();
  my $tokens_p;

  # A session must be established to use the TRANSFER command.
  if ($session_established == 0) {
    $response_key = "547";
    $response = $response_key . $response_codes{$response_key};
    return ($status, $response);
  }

  # Check command structure.
  ($response_key, $tokens_p, $seen_p) = 
  	tokenize(\@$lines, \%transfer_table, \%transfer_elements);
  %tokens = %$tokens_p;
  %seen = %$seen_p;

  # Check validation results.  Check semantics if structure is OK.
  if ($response_key eq "200") {

    # Semantic checks.  Must have a domain name.
    if (!$seen{"domainname"}) {
      $response_key = "504";
    }

    # Must also have an entity name.
    elsif (!$seen{"entityname"}) {
      $response_key = "508";
    }

    # Entity name must be "domain".
    elsif ($tokens{"entityname"} ne "domain") {
      $response_key = "502";
    }

    # Check for known forced error conditions.
    elsif ($seen{"-approve"}) {
      if ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
        $response_key = "534";
      }
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError1"})) {
      $response_key = "536";
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError3"})) {
      $response_key = "545";
    }
    elsif ($tokens{"domainname"} eq lc($cfg->{"DomainError4"})) {
      $response_key = "546";
    }
  }

  # Results.  Optionally create a weighted random response.
  if ($response_key eq "200") {
    $response_key = rand_response(%response_hash);
  }
  $response = $response_key . $response_codes{$response_key};

  return ($status, $response);
}

# =========================================================================
#
# Invalid command response 
#
# =========================================================================
sub invalid_command {
  my ($lines, $cfg) = @_;

  my $response = "";
  my $response_key = "500";
  my $status = 1;

  # Note invalid commands before a session is established and disconnect after
  # a configurable maximum has been met.
  if ($session_established == 0) {
    ++$session_try_count;
    if ($session_try_count >= $cfg->{"MaxSessionAttempts"}) {
      $status = 0;
      $response_key = "520";
    }
  }
  $response = $response_key . $response_codes{$response_key};

  return ($status, $response);
}

# =========================================================================
#
# Utility procedure to remove the command name and command terminator from
# an array of command lines.
#
# =========================================================================
sub shift_pop {
  my ($lines) = @_;

  # Remove the command name from the first array element.
  shift @$lines;

  # Remove the command terminator from the last array element.
  pop @$lines;

  # Return the modified array, now two elements shorter.
  return @$lines;
}

# Utility procedures to generate random responses.
# These are taken from section 2.10 of O'Reilley's "Perl Cookbook".

# =========================================================================
#
# Take a hash mapping key to weight
# and return a hash mapping key to probability.
#
# =========================================================================
sub weight_to_dist {
  my (%weights) = @_;
  my %dist = ();
  my $total = 0;
  my ($key, $weight);
  local $_;

  foreach (values %weights) {
    $total += $_;
  }

  while (($key, $weight) = each %weights) {
    $dist{$key} = $weight/$total;
  }

  return %dist;
}

# =========================================================================
#
# Take a hash mapping key to probability and return a random element.
#
# =========================================================================
sub weighted_rand {
  my (%dist) = @_;
  my ($key, $weight);

  while (1) {
    my $rand = rand;

    while (($key, $weight) = each %dist) {
      return $key if ($rand -= $weight) < 0;
    }
  }
}

# =========================================================================
#
# Return a random response to the client
#
# =========================================================================
sub rand_response {
  my (%hash_table) = @_;
  my %dist = ();
  my $response_key = "";

  %dist = weight_to_dist(%hash_table);
  $response_key = weighted_rand(%dist);
  return $response_key;
}

# Utility procedures to validate values.

# =========================================================================
#
# This procedure validates a domain name.
#
# =========================================================================
sub validate_domain {
  my ($domain) = @_;

  my @elem = ();
  my $response_key = "";
  my $sldn = "";
  my $tld = "";

  # Separate the SLDN from the TLD.
  if ($domain =~ /\./) {
    @elem = split(/\./, $domain);
    $sldn = $elem[0];
    $tld = $elem[1];

    # Validate the TLD.  NSI Registry does public registration for .com, 
    # .net, and .org only.
    if ($tld !~ /^(com|net|org)$/) {
      $response_key = "541";
      return $response_key;
    }
  }
  else {
    $response_key = "541";
    return $response_key;
  }

  # Check the SLDN and return results.
  $response_key = validate_label($sldn);

  return $response_key;
}

# =========================================================================
#
# This procedure validates an entity name.
#
# =========================================================================
sub validate_entity {
  my ($entity) = @_;
  my $response_key = "200";

  # Look for known valid entity names.
  if ($entity !~ /^(domain|nameserver)$/) {
    $response_key = "502";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates an IPv4 address.
#
# =========================================================================
sub validate_ipv4address {
  my ($ipv4address) = @_;

  my $i;
  my $response_key = "200";
  my $packed_ip = 0;

  # Check this address for syntax correctness and convert to 64-bit format.
  if (($packed_ip = inet_to64($ipv4address)) < 0) {
    $packed_ip = validate_ipv6address($ipv4address);
    $response_key = $packed_ip;
    return $response_key;
  }

  # Checked the packed format against the table of restricted addresses.
  # The table contains rows of minimum and maximum values that define a range
  # of addresses.  If this address falls within one of the ranges, it's a
  # restricted address that can't be used for a public name server.
  for $i (0 .. $#ipv4_table) {

    # Check against the min value in the current row.
    if ($packed_ip >= $ipv4_table[$i][0]) {

      # Min value exceeded.  Now check against the max value in the current row.
      if ($packed_ip <= $ipv4_table[$i][1]) {

        # Bingo!  This address IS in one of the ranges!
        $response_key = "535";
        return $response_key;
      }
    }
  }

  # Return normal, "no error found" results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates an IPv6 address.
#
# =========================================================================
sub validate_ipv6address {
  my ($ipv6address) = @_;

  my $i;
  my $response_key = "200";
  my $packed_ip = 0;

  # Check this address for syntax correctness and convert to number format.
  if (($packed_ip = ipv6_tonum($ipv6address)) < 0) {
    $response_key = "541";
    return $response_key;
  }

  # Checked the packed format against the table of restricted addresses.
  # The table contains rows of minimum and maximum values that define a range
  # of addresses.  If this address falls within one of the ranges, it's a
  # restricted address that can't be used for a public name server.
  for $i (0 .. $#ipv6_table) {

    # Check against the min value in the current row.
    if ($packed_ip >= $ipv6_table[$i][0]) {

      # Min value exceeded.  Now check against the max value in the current row.
      if ($packed_ip <= $ipv6_table[$i][1]) {

        # Bingo!  This address IS in one of the ranges!
        $response_key = "535";
        return $response_key;
      }
    }
  }

  # Return normal, "no error found" results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates the label portions of a domain name.
#
# =========================================================================
sub validate_label {
  my ($label) = @_;

  my $response_key = "";

  # Basic rules: a label can contain no more than 63 characters.  A label may
  # start and end with either an alpha character or a digit, and a hyphen may
  # appear in the middle.
  if (length($label) > 63 || length($label) < 1) {
    # Found something with an invalid length.
    $response_key = "541";
  }

  # Length is OK.  Validate the characters that make up the label.  This is done
  # using multiple statements to make the syntax check easier to understand.
  # Start by looking for any invalid characters.  Only alphas, numerics, and
  # hyphens are allowed.
  elsif ($label =~ /[^a-z0-9-]/) {
    $response_key = "505";
  }
  # Now see if the domain name starts or ends with a hyphen.  That's bad.
  elsif ($label =~ /(^-|-$)/) {
    $response_key = "505";
  }
  else {
    # Nothing invalid found -- must be OK.
    $response_key = "200";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates a password string.
#
# =========================================================================
sub validate_password {
  my ($password) = @_;
  my $value_length;
  my $response_key = "200";

  # Check for invalid characters.
  if ($password =~ /[ -~]+/) {

    # Contained characters are OK; check the length.
    $value_length = length($password);

    if ($value_length < 4 || $value_length > 16) {
      # Password is too short or too long.
      $response_key = "506";
    }
  }
  else {
    # Password contains invalid characters.
    $response_key = "506";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates a registration period.
#
# =========================================================================
sub validate_period {
  my ($period) = @_;

  my $response_key = "";

  # A period must be in the range of 1..10 inclusive.
  if ($period >= 1 && $period <= 10) {
    $response_key = "200";
  }
  else {
    $response_key = "541";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates a server name.
#
# =========================================================================
sub validate_server {
  my ($server) = @_;

  my @elem = ();
  my $label = "";
  my $response_key = "";
  my $last_label = "";

  # Check the length of the server string.
  if (length($server) < 1 || length($server) > 80) {
    # Server name is too short or too long.
    $response_key = "506";
    return $response_key;
  }

  # Split the name into labels, check the labels, and return results.
  # Should we do ccTLD validation?
  if ($server =~ /\./) {
    @elem = split(/\./, $server);
    foreach $label (@elem) {
      $response_key = validate_label($label);
      $last_label = $label;
      # Abort if we found an invalid label.
      if ($response_key ne "200") {
        last;
      }
    }

    if($last_label !~ /^(com|net|org)$/) {
      $is_cctldns = 1;
    }
    else {
     $is_cctldns = 0;
    }
  }
  else {
    # Didn't find a ".", so the server name is invalid.
    $response_key = "541";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates a "target".  Right now there's only one 
# valid value.
#
# =========================================================================
sub validate_target {
  my ($target) = @_;
  my $response_key = "200";

  # Look for known valid option values.
  if ($target ne "protocol") {
    $response_key = "506";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
#
#
# =========================================================================
sub validate_word {
  my ($word) = @_;
  my $value_length;
  my $response_key = "200";

  # Check for invalid characters.
  if ($word =~ /^[a-z0-9]{1}[a-z0-9_-]+/) {

    # Contained characters are OK; check the length.
    $value_length = length($word);

    if ($value_length < 1 || $value_length > 128) {
      # Word is too short or too long.
      $response_key = "506";
    }
  }
  else {
    # Word contains invalid characters.
    $response_key = "506";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates a year value.
#
# =========================================================================
sub validate_year {
  my ($year) = @_;
  my $response_key = "200";

  # Check for invalid characters.
  if ($year !~ /^\d{1,4}$/) {
    # Found something other than a four-digit year.
    $response_key = "506";
  }

  elsif ($year !~ /^(19|20)/) {
    # The year value should start with either 19 or 20.
    $response_key = "506";
  }

  # Return results.
  return $response_key;
}

# =========================================================================
#
# This procedure validates a "yes/no" string.
#
# =========================================================================
sub validate_yesno {
  my ($yesno) = @_;
  my $response_key = "200";

  # Check for invalid characters.
  if ($yesno !~ /^(yes|no)$/) {
    # Found something other than "yes" or "no".
    $response_key = "506";
  }

  # Return results.
  return $response_key;
}

# Utility procedures to validate command structure.

# =========================================================================
# This procedure does some reformatting of the command array (it removes the
# command name and end-of-command terminator) and calls syntax validation
# procedures to check the structural validity of command elements.  It returns
# a response key, a hash table of parsed tokens and values, and a hash table
# of tokens and entry counts.
# =========================================================================
sub tokenize {
  my ($lines, $command_hash, $element_hash) = @_;

  my %commands = %$command_hash;
  my @elem = ();
  my %elements = %$element_hash;
  my $line = "";
  my $response_key = "200";
  my %seen = ();
  my %tokens = ();

  # Examine each line of the command.
  # First remove the command name from the first element of the array and
  # the command terminator from the last element of the array.
  @$lines = shift_pop(\@$lines);

  # Count the number of instances of each line in the command.
  # Split the line into a token and value.
  foreach $line (@$lines) {
    @elem = split(/:/, $line, 2);

    # See if the token represents a known command element.
    if ($commands{$elem[0]}) {

      # Remove processing flags from the value, if present.
      if (exists $elements{"flag"}) {
        my $flag = $elements{"flag"};

        if ($elem[1] =~ /$flag$/) {
          chop($elem[1]);
        }
      }

      # first determine if there is a value to validate
      if (defined $elem[1] && $elem[1] ne "") {
        # Found a known command element.  Validate it.
        if (($response_key = $commands{$elem[0]}->($elem[1])) ne "200") {

          # The value element isn't valid.  Quit now.
          last;
        }
      }
      else {
        # ok invalid command format
        $response_key = "507";
        last;
      }

      # Found a valid token and value.  Note it.
      ++$seen{$elem[0]};

      # In general, multiple occurrences of a token are invalid.  However, some
      # commands will accept multiple occurrences of a "NameServer" token.  
	  # Don't flag those as an error.  In fact, knowing how many are provided 
      # is needed.
      if ($seen{$elem[0]} > 1 &&
         ($elem[0] ne "nameserver" && $elem[0] ne "ipaddress")) {
        $response_key = "507";
        last;
      }
      $tokens{$elem[0]} = $elem[1];
    }

    else {
      # Found an unknown command element.  Reject it.
      if ($elem[0] =~ /^-/) {
        # Found an unrecognized command option.  The appropriate error code 
        # depends on whether or not this command expects any options.
        if (exists $elements{"options"}) {
          $response_key = "501";
        }
        else {
          $response_key = "507";
        }
      }
      else {
        # Found something other than an option.  The appropriate error code 
        # depends on whether or not this command expects attributes and what 
        # the invalid thing looks like.
        if (exists $elements{"attributes"}) {
          # Does it look like an attribute/value pair?
          if ($line =~ /.+:.+/) {
            $response_key = "503";
          }
          else {
            $response_key = "507";
          }
        }
        # Command does not expect any attributes.
        else {
          $response_key = "507";
        }
      }
      last;
    }
  }

  # Return results.
  return ($response_key, \%tokens, \%seen);
}

# =========================================================================
#
# Return the response string for a given response key.
#
# =========================================================================
sub get_response {
  my ($response_key) = @_;
  return $response_key . $response_codes{$response_key};
}

# Last line.
1;
