package main;

use strict;
use warnings;

$HMConfig::culHmSubTypeSets{"iButton"} = { peerChan => "-btnNumber- -actChn- ... single [set|unset] [actor|remote|both]",
                                           on => "" };
$HMConfig::culHmSubTypeSets{"Values"} = { };

$HMConfig::culHmRegDefine{"addressHi"} = {a=>224,s=>4,l=>1,min=>0,max=>0xffffffff,c=>'hex',f=>'',u=>'',d=>1,t=>"address high"};
$HMConfig::culHmRegDefine{"addressLo"} = {a=>228,s=>4,l=>1,min=>0,max=>0xffffffff,c=>'hex',f=>'',u=>'',d=>1,t=>"address low"};

$HMConfig::culHmRegType{ibutton} = { peerNeedsBurst=>1, expectAES=>1, addressHi=>1, addressLo=>1 };
$HMConfig::culHmRegType{values} = { };

$HMConfig::culHmModel{"F201"} = {name=>"HM-LC-Sw2-FM-CustomFW",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:3p.4p,4:1p.2p',chn=>"Btn:1:2,Sw:3:4"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW02"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW04"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmRegChan {"HM-LC-Sw2-FM-CustomFW01"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HM-LC-Sw2-FM-CustomFW02"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HM-LC-Sw2-FM-CustomFW03"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HM-LC-Sw2-FM-CustomFW04"} = $HMConfig::culHmRegType{switch};

$HMConfig::culHmModel{"F202"} = {name=>"HB-SW2-SENS",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p,4:3p',chn=>"Sw:1:2,Sen:3:3"};
$HMConfig::culHmChanSets{"HB-SW2-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-SW2-SENS01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS03"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegChan {"HB-SW2-SENS01"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-SW2-SENS02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-SW2-SENS03"} = $HMConfig::culHmRegType{threeStateSensor};

$HMConfig::culHmModel{"F203"} = {name=>"HB-DoorBell",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:8p,4:1p.2p.3p.4p.5p.6p.7p',chn=>"Bell:1:1,Btn:2:7,Sw:8:8,Values:9:9"};
$HMConfig::culHmChanSets{"HB-DoorBell00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-DoorBell01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-DoorBell02"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell03"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell04"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell05"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell06"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell07"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell08"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-DoorBell09"} = $HMConfig::culHmSubTypeSets{"Values"};
$HMConfig::culHmRegChan {"HB-DoorBell00"} = { pairCentral=>1, backOnTime=>1 };
$HMConfig::culHmRegChan {"HB-DoorBell01"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-DoorBell02"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell03"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell04"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell05"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell06"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell07"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell08"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-DoorBell09"} = $HMConfig::culHmRegType{values};

$HMConfig::culHmModel{"F205"} = {name=>"HB-GEN-SENS",st=>'custom',cyc=>'',rxt=>'c',lst=>'1',chn=>"Values:1:1"};
$HMConfig::culHmChanSets{"HB-GEN-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-GEN-SENS01"} = $HMConfig::culHmSubTypeSets{"Values"};
$HMConfig::culHmRegChan {"HB-GEN-SENS01"} = $HMConfig::culHmRegType{values};


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


sub CUL_HM_Parsecustom($$$$$$) {
  my($mFlg,$mTp,$src,$dst,$p,$target) = @_;
  my @evtEt = ();

#  Log 1,"General  entering with $mFlg,$mTp,$src,$dst,$p,$target";
#  Log 1, $src;
#  Log 1, CUL_HM_id2Hash($src)->{NAME};
  my $model = AttrVal(CUL_HM_id2Hash($src)->{NAME},"model","");
#  Log 1, $model;

  # handle Ack_Status or Info_Status message here
  if (($mTp eq "02" && $p =~ m/^01/) || ($mTp eq "10" && $p =~ m/^06/) ) {
    # split payload
    my ($subType,$chn,$val,$err) = ($1,hex($2),hex($3)/2,hex($4))
                        if($p =~ m/^(..)(..)(..)(..)/);
    $chn = sprintf("%02X",$chn&0x3f);
	my $chId = $src.$chn;
    my $shash = $modules{CUL_HM}{defptr}{$chId}
                    if($modules{CUL_HM}{defptr}{$chId});
	
    # update state of a switch  
    if( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{switch} ) {
      Log 1, $model.$chn." is switch";
      # user string
      my $vs = ($val==100 ? "on":($val==0 ? "off":"$val %"));

      push @evtEt,[$shash,1,"level:$val %"];
      push @evtEt,[$shash,1,"pct:$val"];      # duplicate to level - necessary for "slider"
      push @evtEt,[$shash,1,"deviceMsg:$vs$target"] if($chn ne "00");
      push @evtEt,[$shash,1,"state:".$vs];
      push @evtEt,[$shash,1,"timedOn:".(($err&0x40)?"running":"off")];
    }
    elsif( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{threeStateSensor} ) {
      Log 1, $model.$chn." is threeStateSensor";
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$chn};
      my $vs = ($val==100 ? "open" : ($val==0 ? "closed" : "tilted"));
      push @evtEt,[$chnHash,1,"state:".$vs];
	  push @evtEt,[$chnHash,1,"contact:$vs$target"];
	}
  }
  # handle sensor event
  elsif($mTp =~ m/^41/ && $p =~ m/^(..)(..)(..)/) {
    my $shash = CUL_HM_id2Hash($src);
    my ($chn,$cnt,$val) = (hex($1),$2,hex($3)/2);
	$chn = sprintf("%02X",$chn & 0x3f);
    # Log 1, "41 ".$model.$chn." ".$val;
    if( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{threeStateSensor} ) {
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$chn};
      Log 1, $model.$chn." is threeStateSensor";
      my $vs = ($val==100 ? "open" : ($val==0 ? "closed" : "tilted"));
      push @evtEt,[$chnHash,1,"state:".$vs];
	  push @evtEt,[$chnHash,1,"contact:$vs$target"];
	}
  }
  # handle remote event
  elsif($mTp =~ m/^40/ && $p =~ m/^(..)(..)/) {
    my $shash = CUL_HM_id2Hash($src);
    my ($chn, $bno) = (hex($1), hex($2));# button number/event count
    my $buttonID = sprintf("%02X",$chn&0x3f);# only 6 bit are valid

    # update state of a remote  
    if( $HMConfig::culHmRegChan{$model.$buttonID} == $HMConfig::culHmRegType{remote} ) {
      Log 1, $model.$buttonID." is remote";
      my $btnName;
      my $state = "";
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$buttonID};

      if ($chnHash){# use userdefined name - ignore this irritating on-off naming
        $btnName = $chnHash->{NAME};
      }
      else{# Button not defined, use default naming
        $chnHash = $shash;
        my $btn = int((($chn&0x3f)+1)/2);
        $btnName = "Btn$btn";
        $state = ($chn&1 ? "off" : "on")
      }
      my $trigType;
      if($chn & 0x40){
        if(!$shash->{BNO} || $shash->{BNO} ne $bno){#bno = event counter
          $shash->{BNO}=$bno;
          $shash->{BNOCNT}=0; # message counter reest
        }
        $shash->{BNOCNT}+=1;
        $state .= "Long" .((hex($mFlg) & 0x24)==0x20 ? "Release" : "").
                  " ".$shash->{BNOCNT};
        $trigType = "Long";
      }
      else{
        $state .= "Short";
        $trigType = "Short";
      }
      $shash->{helper}{addVal} = $chn;   #store to handle changesFread
      push @evtEt,[$chnHash,1,"state:".$state.$target];
      push @evtEt,[$chnHash,1,"trigger:".$trigType."_".$bno];
      push @evtEt,[$shash,1,"state:$btnName $state$target"];
    }
  }
  # handle generic value event
  elsif($mTp =~ m/^53/ && $p =~ m/^(..)(..)(.*)/) {
    my $shash = CUL_HM_id2Hash($src);
    my ($chn,$numval) = (hex($1), hex($2)); # read channel and number of values
	my $values = $3;
    my $chnnum = sprintf("%02X",$chn&0x3f);# only 6 bit are valid
	
#	2018.04.15 21:56:46 1: HB-GEN-SENS01 has 4 values (090400E82803EB0000)
#	Log(1, "Values Message for Channel: $chnnum");
    if( $HMConfig::culHmRegChan{$model.$chnnum} == $HMConfig::culHmRegType{values} ) {
      Log 1, $model.$chnnum." has $numval values ($p)";
	  my $chnHash = $modules{CUL_HM}{defptr}{$src.$chnnum};
      my $vfmt = AttrVal($chnHash->{NAME},"valuesformat","");
	  # Log 1,$vfmt;
	  if( $vfmt eq "" ) {
	    Log 1, "Missing attribute valuesformat at $chnHash->{NAME}";
		for( my $i=0; $i<$numval; ++$i ) {
  		  $vfmt = $vfmt."1 ";
		}
	  }
	  my @valuesfmt = parseValueFormat($vfmt);
	  if( $numval != scalar(@valuesfmt) ) {
	    Log 1, "Attribute valuesformat mismatch at $chnHash->{NAME} - expected $numval items but got ".scalar(@valuesfmt)." items";
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
        push @evtEt,[$chnHash,1,$data->{'reading'}.":".$val];
	    $state = $state.$val." ";
      }
      push @evtEt,[$chnHash,1,"state:".$state];
	}
  } else {
    Log(1, "HMConfig_AskSinPPCustom received unknown message: $mFlg,$mTp,$src,$dst,$p");
  }

  return @evtEt;
}

1;
