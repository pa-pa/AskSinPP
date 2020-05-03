
package HMMsg;

use strict;
use warnings;

sub new {
  my $class = shift;
  my $self = { _type => hex(shift),
               _flags => hex(shift),
               _from => shift,
               _to => shift,
               _payload => shift  };
  bless $self, $class;
  return $self;
}

# print the message
sub print {
  my ($self) = @_;
  print $self->{_type}." ".$self->{_flags}." ".$self->{_from}." ".$self->{_to}." ".$self->{_payload}."\n";
}
# payload
# payload OFFSET
# payload OFFSET,LENGTH
sub payload {
  my ($self,$offset,$length) = @_;
  return substr($self->{_payload},$offset,$length) if defined($length);
  return substr($self->{_payload},$offset) if defined($offset);
  return $self->{_payload};
}
# payloadByte OFFSET
sub payloadByte {
  my ($self,$offset) = @_;
  return hex($self->payload($offset*2,2));
}
# payloadWord OFFSET
sub payloadWord {
  my ($self,$offset) = @_;
  return hex($self->payload($offset*2,4));
}
# payload3Byte OFFSET
sub payload3Byte {
  my ($self,$offset) = @_;
  return hex($self->payload($offset*2,6));
}
# payloadLong OFFSET
sub payloadLong {
  my ($self,$offset) = @_;
  return hex($self->payload($offset*2,8));
}
sub from {
  my ($self) = @_;
  return $self->{_from};
}
sub to {
  my ($self) = @_;
  return $self->{_to};
}
sub flags {
  my ($self) = @_;
  return $self->{_flags};
}
sub type {
  my ($self) = @_;
  return $self->{_type};
}

sub isStatus {
  my ($self) = @_;
  my $byte0 = $self->payloadByte(0);
  return ($self->type == 0x2 && $byte0 == 0x1) || ($self->type == 0x10 && $byte0 == 0x6);
}
sub isSensor {
  my ($self) = @_;
  return $self->type == 0x41;  
}
sub isRemote {
  my ($self) = @_;
  return $self->type == 0x40;  
}
sub isValues {
  my ($self) = @_;
  return $self->type == 0x53;
}

sub isRemoteLong {
  my ($self) = @_;
  return $self->payloadByte(0) & 0x40;
}
sub isRemoteLowBat {
  my ($self) = @_;
  return $self->payloadByte(0) & 0x80;
}

sub channel {
  my ($self) = @_;
  my $chn;
  if( $self->isStatus ) {
    $chn = $self->payloadByte(1) 
  }
  else {
    $chn = $self->payloadByte(0);
  }
  return ($chn & 0x3f);
}

sub channelId {
  my ($self,$cnum) = @_;
  if( ! defined($cnum) ) {
    $cnum = $self->channel;
  }
  return $self->from . sprintf("%02X",$cnum);
}
  
sub processSwitchStatus {
  my ($self,$target) = @_;
  my @evtEt=();
  my $channel = $main::modules{CUL_HM}{defptr}{$self->channelId};
  if( defined($channel) ) {
    my $value = $self->payloadByte(2)/2;
    my $flags = $self->payloadByte(3);
    my $valuestr = "$value %";
    $valuestr = "on"  if $value==100;
    $valuestr = "off" if $value==0;

    push @evtEt,[$channel,1,"level:$value %"];
    push @evtEt,[$channel,1,"pct:$value"];     # duplicate to level - necessary for "slider"
    push @evtEt,[$channel,1,"deviceMsg:$valuestr$target"];
    push @evtEt,[$channel,1,"state:".$valuestr];
    push @evtEt,[$channel,1,"timedOn:".(($flags & 0x40)?"running":"off")];
  }
  else {
    main::Log 1,"No object for ".$self->channelId;
  }
  return @evtEt;
}

sub processBlindStatus {
  my ($self,$target,$channel) = @_;
  my @evtEt=();
  # no channel given - calc channel
  if( ! defined($channel) ) {
    $channel = $main::modules{CUL_HM}{defptr}{$self->channelId};
  }
  if( defined($channel) ) {
    my $value = $self->payloadByte(2)/2;
    my $flags = $self->payloadByte(3);
    my $valuestr = "$value %";
    $valuestr = "on"  if $value==100;
    $valuestr = "off" if $value==0;
    my $dir = ($flags >> 4) & 3;
    my %dirName = ( 0=>"stop" ,1=>"up" ,2=>"down" ,3=>"err" );
    push @evtEt,[$channel,1,"level:$value"];
    push @evtEt,[$channel,1,"motor:$dirName{$dir}:$valuestr"];
    push @evtEt,[$channel,1,"pct:$value"];     # duplicate to level - necessary for "slider"
    push @evtEt,[$channel,1,"deviceMsg:$valuestr$target"];
    push @evtEt,[$channel,1,"state:".$valuestr];
    push @evtEt,[$channel,1,"timedOn:".(($flags & 0x40)?"running":"off")];
  }
  else {
    main::Log 1,"No object for ".$self->channelId;
  }
  return @evtEt;
}

sub processMotion {
  my ($self,$target) = @_;
  my @evtEt=();
  my $channel = $main::modules{CUL_HM}{defptr}{$self->channelId};
  if( defined($channel) ) {
  	if( $self->isStatus ) {
      my $bright = $self->payloadByte(2);
      push @evtEt,[$channel,1,"brightness:$bright"];
  	}
  	if( $self->isSensor ) {
      my $cnt = $self->payloadByte(1);
      my $bright = $self->payloadByte(2);
      my $next = $self->payloadByte(3);
      if( $next ) {
        my $stamp =  ::gettimeofday(); # take reception time;
        $next = (15 << ($next >> 4) - 4); # strange mapping of literals
        main::RemoveInternalTimer($channel->{NAME}.":motionCheck");
        main::InternalTimer($stamp+$next+2,"CUL_HM_motionCheck", $channel->{NAME}.":motionCheck", 0);
        $channel->{helper}{moStart} = $stamp if (!defined $channel->{helper}{moStart});
      }
      else {
        $next = "none";
      }
      push @evtEt,[$channel,1,"state:motion"];
      push @evtEt,[$channel,1,"motion:on$target"];
      push @evtEt,[$channel,1,"motionCount:$cnt"."_next:$next"."s"];
      push @evtEt,[$channel,1,"brightness:$bright"];
  	}
  }
  else {
    main::Log 1,"No object for ".$self->channelId;
  }
  return @evtEt;
}

sub processRemote {
  my ($self,$target) = @_;
  my @evtEt=();
  my $device = main::CUL_HM_id2Hash($self->from);
  my $channel = $main::modules{CUL_HM}{defptr}{$self->channelId};
  my $bno = $self->payloadByte(1);
  if( defined($channel) ) {
    my $state = "Short";
    my $trigType = "Short";
    my $btnName = $channel->{NAME};
    if( $self->isRemoteLong ) {
      if(!$device->{BNO} || $device->{BNO} ne $bno) { #bno = event counter
        $device->{BNO} = $bno;
        $device->{BNOCNT} = 0; # message counter reest
      }
      $device->{BNOCNT} += 1;
      $state = "Long" .(($self->flags & 0x24)==0x20 ? "Release" : "")." ".$device->{BNOCNT};
      $trigType = "Long";
    }
    $device->{helper}{addVal} = $self->payloadByte(0);   #store to handle changesFread
    push @evtEt,[$channel,1,"state:".$state.$target];
    push @evtEt,[$channel,1,"trigger:".$trigType."_".$bno];
    push @evtEt,[$device,1,"state:$btnName $state$target"];
  }
  else {
    main::Log 1,"No object for ".$self->channelId;
  }
  return @evtEt;
}

sub processThreeState {
  my ($self,$target,%mapping) = @_;
  my @evtEt=();
  my $channel = $main::modules{CUL_HM}{defptr}{$self->channelId};
  if( ! defined($channel) ) {
  	# fallback to device
  	$channel = main::CUL_HM_id2Hash($self->from);
  }
  if( ! %mapping ) {
    %mapping = (0=>'closed',100=>'tilted',200=>'open');
  }
  my $val = $self->payloadByte(2);
  if( defined($channel) ) {
    my $vs = $val/2;
    $vs = $mapping{$val} if defined $mapping{$val};
    push @evtEt,[$channel,1,"state:".$vs];
    push @evtEt,[$channel,1,"contact:$vs$target"];
  }
  else {
    main::Log 1,"No object for ".$self->channelId;
  }
  return @evtEt;
}

sub parseValueFormat {
  my @v;
  foreach my $value ( split / /,$_[0] ) {
    #print $value."\n";
    my @parts = split /:/,$value;
    my $valuedata = {};
    my $numb = $parts[0];
    $numb =~ s/([1,2,4,8])s?/$1/g;
    $valuedata->{'numbytes'} = $numb;
    $valuedata->{'signed'} = $parts[0] =~ m/([1,2,4,8])s/;
    $valuedata->{'reading'} = "value".scalar @v + 1;
    $valuedata->{'factor'} = 1;
    if( defined $parts[1] ) { $valuedata->{'reading'} = $parts[1]; }
    if( defined $parts[2] ) { $valuedata->{'factor'} = $parts[2]; }
    push @v, $valuedata;
  }
  return @v;
}

sub processValues {
  my ($self,$target) = @_;
  my @evtEt=();
  my $channel = $main::modules{CUL_HM}{defptr}{$self->channelId};
  my $numval = $self->payloadByte(1); # get number of values from message
  my $values = $self->payload(4);     # values starting at byte 4

  my $vfmt = main::AttrVal($channel->{NAME},"valuesformat","");
  # Log 1,$vfmt;
  if( $vfmt eq "" ) {
    main::Log3 $channel->{NAME}, 1, "Missing attribute valuesformat at $channel->{NAME}";
    for( my $i=0; $i<$numval; ++$i ) {
      $vfmt = $vfmt."1 ";
    }
  }
  my @valuesfmt = parseValueFormat($vfmt);
  my $fmtnumval = scalar(@valuesfmt);
  if( $numval != $fmtnumval ) {
    main::Log3 $channel->{NAME}, 1, "Attribute valuesformat mismatch at $channel->{NAME} - expected $numval items but got $fmtnumval items";
  }
  my $packstr = "";
  my $state = "";
  foreach my $data (@valuesfmt) {
    #Log 1, $data->{'numbytes'}."  ".$data->{'signed'}."  ".$data->{'reading'}."  ".$data->{'factor'}."\n";
    $packstr = $packstr."A".($data->{'numbytes'}*2);
  }
  #print $packstr."\n";
  my @unpacked = map{hex($_)} unpack($packstr,$values);
  #print "$unpacked[0] $unpacked[1] $unpacked[2] $unpacked[3]\n";
  my $num = 0;
  foreach my $data (@valuesfmt) {
    my $val = $unpacked[$num++];
    if( $data->{'signed'} ) {
      my $max =  0x01 << (8*$data->{'numbytes'});
      my $mask = $max >> 1;
      $val = ($val & $mask) ? ($val - $max) : $val;
    }
    $val /= $data->{'factor'};
    # print $data->{'reading'}." : ".$val."\n";
    push @evtEt,[$channel,1,$data->{'reading'}.":".$val];
    $state = $state.$val." ";
  }
  push @evtEt,[$channel,1,"state:".$state];
  return @evtEt;
}

sub ::HMMsg_Initialize {}
  
1;
