package main;

use strict;
use warnings;

$HMConfig::culHmModel{"F201"} = {name=>"HM-LC-Sw2-FM-CustomFW",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:3p.4p,4:1p.2p',chn=>"Btn:1:2,Sw:3:4"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW02"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HM-LC-Sw2-FM-CustomFW04"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmRegChan{"HM-LC-Sw2-FM-CustomFW01"}  = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan{"HM-LC-Sw2-FM-CustomFW02"}  = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan{"HM-LC-Sw2-FM-CustomFW03"}  = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan{"HM-LC-Sw2-FM-CustomFW04"}  = $HMConfig::culHmRegType{switch};

$HMConfig::culHmModel{"F202"} = {name=>"HB-SW2-SENS",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p,4:3p',chn=>"Sw:1:2,Sen:3:3"};
$HMConfig::culHmChanSets{"HB-SW2-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-SW2-SENS01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS03"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegChan{"HB-SW2-SENS01"}  = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan{"HB-SW2-SENS02"}  = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan{"HB-SW2-SENS03"}  = $HMConfig::culHmRegType{threeStateSensor};


sub CUL_HM_Parsecustom($$$$$$) {
  my($mFlg,$mTp,$src,$dst,$p,$target) = @_;
  my @evtEt = ();

#  Log 1,"General  entering with $mFlg,$mTp,$src,$dst,$p";
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
        $state .= "Long" .(hex($mFlg) & 0x20 ? "Release" : "").
                  " ".$shash->{BNOCNT}."-".$mFlg.$mTp."-";
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
  } else {
    Log(1, "AskSinPP_HM_LC_Sw2_FM_CustomFW received unknown message: $mFlg,$mTp,$src,$dst,$p");
  }

  return @evtEt;
}

1;
