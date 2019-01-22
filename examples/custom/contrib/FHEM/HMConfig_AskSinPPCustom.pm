package main;

use strict;
use warnings;

$HMConfig::culHmSubTypeSets{"iButton"} = { peerChan => "-btnNumber- -actChn- ... single [set|unset] [actor|remote|both]",
                                           on => "" };
$HMConfig::culHmSubTypeSets{"Values"}  = { };

# iButton Channels
$HMConfig::culHmRegDefine{"addressHi"}  = {a=>224,s=>4,l=>1,min=>0,max=>0xffffffff,c=>'hex',p=>'n',f=>'',u=>'',d=>1,t=>"address high"};
$HMConfig::culHmRegDefine{"addressLo"}  = {a=>228,s=>4,l=>1,min=>0,max=>0xffffffff,c=>'hex',p=>'n',f=>'',u=>'',d=>1,t=>"address low"};
$HMConfig::culHmRegDefine{"buttonMode"} = {a=>50 ,s=>1,l=>0,min=>0,max=>1,         c=>'lit',p=>'n',f=>'',u=>'',d=>1,t=>"button mode",lit=>{remote=>0,state=>1}};

# HB-UNI-Sen-LEV-US
$HMConfig::culHmRegDefine{"sendInterval"}   = {a=>32, s=>2,l=>0,min=>10,max=>3600, c=>'',   p=>'n',f=>'',u=>'s', d=>1,t=>"send interval"};
$HMConfig::culHmRegDefine{"distanceOffset"} = {a=>1 , s=>2,l=>1,min=>0,max=>600,   c=>'',   p=>'n',f=>'',u=>'cm',d=>1,t=>"distance offset"};
$HMConfig::culHmRegDefine{"sensorType"}     = {a=>3 , s=>1,l=>1,min=>0,max=>1,     c=>'lit',p=>'n',f=>'',u=>'',  d=>1,t=>"sensor type",lit=>{jsn=>0,maxsonar=>1}};
$HMConfig::culHmRegDefine{"caseDesign"}     = {a=>90, s=>1,l=>1,min=>0,max=>2,     c=>'lit',p=>'n',f=>'',u=>'',  d=>1,t=>"case design",lit=>{vertical=>0,horizontal=>1,rectangle=>2}};
$HMConfig::culHmRegDefine{"caseHeight"}     = {a=>94, s=>2,l=>1,min=>30,max=>10000,c=>'',   p=>'n',f=>'',u=>'cm',d=>1,t=>"case height"};
$HMConfig::culHmRegDefine{"caseWidth"}      = {a=>102,s=>2,l=>1,min=>30,max=>10000,c=>'',   p=>'n',f=>'',u=>'cm',d=>1,t=>"case width"};
$HMConfig::culHmRegDefine{"caseLength"}     = {a=>106,s=>2,l=>1,min=>30,max=>10000,c=>'',   p=>'n',f=>'',u=>'cm',d=>1,t=>"case length"};

$HMConfig::culHmRegType{ibutton}   = { peerNeedsBurst=>1, expectAES=>1, addressHi=>1, addressLo=>1 };
$HMConfig::culHmRegType{values}    = { eventDlyTime=>1 };

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

$HMConfig::culHmModel{"F335"} = {name=>"HM-LC-Sw1-FM-Cus",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:3p,3:1p.2p',chn=>"Btn:1:2,Sw:3:3"};
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus02"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmRegChan {"HM-LC-Sw1-FM-Cus01"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HM-LC-Sw1-FM-Cus02"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HM-LC-Sw1-FM-Cus03"} = $HMConfig::culHmRegType{switch};

$HMConfig::culHmModel{"F202"} = {name=>"HB-SW2-SENS",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p,4:3p',chn=>"Sw:1:2,Sen:3:3"};
$HMConfig::culHmChanSets{"HB-SW2-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-SW2-SENS01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS03"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegChan {"HB-SW2-SENS01"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-SW2-SENS02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-SW2-SENS03"} = $HMConfig::culHmRegType{threeStateSensor};

$HMConfig::culHmModel{"F203"} = {name=>"HB-DoorBell",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:2p.3p.5p,4:1p.6p.7p.8p.9p.10p.11p',chn=>"Bell:1:1,Sw:2:2,Ir:3:3,Values:4:4,Motion:5:5,Id:6:11"};
$HMConfig::culHmChanSets{"HB-DoorBell00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-DoorBell01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-DoorBell02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-DoorBell03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-DoorBell04"} = $HMConfig::culHmSubTypeSets{"Values"};
$HMConfig::culHmChanSets{"HB-DoorBell05"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-DoorBell06"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell07"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell08"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell09"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell0A"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-DoorBell0B"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmRegChan {"HB-DoorBell"}   = { pairCentral=>1, backOnTime=>1 };
$HMConfig::culHmRegChan {"HB-DoorBell01"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-DoorBell02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-DoorBell03"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-DoorBell04"} = $HMConfig::culHmRegType{values};
$HMConfig::culHmRegChan {"HB-DoorBell05"} = $HMConfig::culHmRegType{motionDetector};
$HMConfig::culHmRegChan {"HB-DoorBell06"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell07"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell08"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell09"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell0A"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-DoorBell0B"} = $HMConfig::culHmRegType{ibutton};

$HMConfig::culHmModel{"F205"} = {name=>"HB-GEN-SENS",st=>'custom',cyc=>'',rxt=>'c',lst=>'1',chn=>"Values:1:1"};
$HMConfig::culHmChanSets{"HB-GEN-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-GEN-SENS01"} = $HMConfig::culHmSubTypeSets{"Values"};
$HMConfig::culHmRegChan {"HB-GEN-SENS01"} = $HMConfig::culHmRegType{values};

$HMConfig::culHmModel{"F206"} = {name=>"HB-IBUT-8",st=>'custom',cyc=>'',rxt=>'',lst=>'1,4',chn=>"Id:1:8"};
$HMConfig::culHmChanSets{"HB-IBUT-800"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-IBUT-801"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-802"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-803"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-804"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-805"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-806"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-807"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmChanSets{"HB-IBUT-808"} = $HMConfig::culHmSubTypeSets{"iButton"};
$HMConfig::culHmRegChan {"HB-IBUT-8"}   = { pairCentral=>1, buttonMode=>1 };
$HMConfig::culHmRegChan {"HB-IBUT-801"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-802"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-803"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-804"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-805"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-806"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-807"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-808"} = $HMConfig::culHmRegType{ibutton};

$HMConfig::culHmModel{"F9D2"} = {name=>"HB-UNI-Sen-LEV-US",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1',chn=>"Level:1:1"};
$HMConfig::culHmChanSets{"HB-UNI-Sen-LEV-US00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-Sen-LEV-US01"} = {};
$HMConfig::culHmRegChan {"HB-UNI-Sen-LEV-US"}   = { pairCentral=>1, lowBatLimit=>1, sendInterval=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-LEV-US01"} = { distanceOffset=>1, sensorType=>1, caseDesign=>1, caseHeight=>1, caseWidth=>1, caseLength=>1 };

#Custom registers for HB-LC-Dim1PBU-FM
for my $reading0 (keys %{$HMConfig::culHmRegType{dimmer}}) {
	$HMConfig::culHmRegType{custom_dimmer}{$reading0} = 1;
}
for my $reading1 (keys %{$HMConfig::culHmRegModel{"HM-LC-Dim1TPBU-FM"}}) {
		$HMConfig::culHmRegType{custom_dimmer}{$reading1} = 1;
}
$HMConfig::culHmModel{"F299"} = {name=>"HB-LC-Dim1PBU-FM",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:3p.4p.5p,4:1p.2p',chn=>"Btn:1:2,Dim:3:3,Dim_V:4:5"};
$HMConfig::culHmChanSets{"HB-LC-Dim1PBU-FM00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-LC-Dim1PBU-FM01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-LC-Dim1PBU-FM02"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-LC-Dim1PBU-FM03"} = $HMConfig::culHmSubTypeSets{"dimmer"};
$HMConfig::culHmChanSets{"HB-LC-Dim1PBU-FM04"} = $HMConfig::culHmSubTypeSets{"dimmer"};
$HMConfig::culHmChanSets{"HB-LC-Dim1PBU-FM05"} = $HMConfig::culHmSubTypeSets{"dimmer"};
$HMConfig::culHmRegChan {"HB-LC-Dim1PBU-FM01"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-LC-Dim1PBU-FM02"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-LC-Dim1PBU-FM03"} = $HMConfig::culHmRegType{custom_dimmer};
$HMConfig::culHmRegChan {"HB-LC-Dim1PBU-FM04"} = $HMConfig::culHmRegType{custom_dimmer};
$HMConfig::culHmRegChan {"HB-LC-Dim1PBU-FM05"} = $HMConfig::culHmRegType{custom_dimmer};


sub HMConfig_AskSinPPCustom_Initialize () {
  # Log 1,"Rerun Literal reverse mapping";
  foreach my $rN  (keys %HMConfig::culHmRegDefine){ #create literal inverse for fast search
    if ($HMConfig::culHmRegDefine{$rN}{lit}){# literal assigned => create inverse
      foreach my $lit (keys %{$HMConfig::culHmRegDefine{$rN}{lit}}){
        $HMConfig::culHmRegDefine{$rN}{litInv}{$HMConfig::culHmRegDefine{$rN}{lit}{$lit}}=$lit;
      }
    }
  }
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
    my ($subType,$chn,$val,$err,$xx,$pl);
    if($p =~ m/^(..)(..)(..)(..)/){
       ($subType,$chn,$val,$err) = ($1,hex($2),hex($3)/2,hex($4));
    }
    if($p =~ m/^(..)(..)(..)(..)(..)(..)/){
        ($subType,$chn,$val,$err,$xx,$pl) = ($1,hex($2),hex($3)/2,hex($4),hex($5),hex($6)/2);
    }
    $chn = sprintf("%02X",$chn&0x3f);
  	my $chId = $src.$chn;
    my $shash = $modules{CUL_HM}{defptr}{$chId}
                    if($modules{CUL_HM}{defptr}{$chId});
	
    # update state of a switch  
    if( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{switch} ) {
      Log3 $shash->{NAME}, 4, $model.$chn." is switch";
      # user string
      my $vs = ($val==100 ? "on":($val==0 ? "off":"$val %"));
      push @evtEt,[$shash,1,"level:$val %"];
      push @evtEt,[$shash,1,"pct:$val"];      # duplicate to level - necessary for "slider"
      push @evtEt,[$shash,1,"deviceMsg:$vs$target"] if($chn ne "00");
      push @evtEt,[$shash,1,"state:".$vs];
      push @evtEt,[$shash,1,"timedOn:".(($err&0x40)?"running":"off")];
    }
    elsif( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{threeStateSensor} ) {
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$chn};
      Log3 $chnHash->{NAME}, 4, $model.$chn." is threeStateSensor";
      my $vs = ($val==100 ? "open" : ($val==0 ? "closed" : "tilted"));
      push @evtEt,[$chnHash,1,"state:".$vs];
	    push @evtEt,[$chnHash,1,"contact:$vs$target"];
	  }
  # update state of a dimmer
     if( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{dimmer} ||  $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{custom_dimmer}) {
	 	 
	my $mId = CUL_HM_getMId($shash);
	my $chnRemMax;
	my $rSUpdt = 0;# require status update
	my $cName = CUL_HM_id2Name($src.sprintf("%02X",$chn));
    my($lvlMin,$lvlMax)=split",",AttrVal($cName, "levelRange", "0,100");
	my $physLvl; 
	my $vDim = $shash->{helper}{vDim}; #shortcut
	
	#do we have remote channels ?
	if ($HMConfig::culHmModel{$mId} && $HMConfig::culHmModel{$mId}{chn} =~ m/Btn/){
		my @chnRem = (grep{$_ =~ m/Btn:/ } split ',',$HMConfig::culHmModel{$mId}{chn});
		@chnRem = split ':',$chnRem[0] if (@chnRem);
		$chnRemMax = $chnRem[2];
	}
	else{
		$chnRemMax = 0;
	}
	if ($HMConfig::culHmModel{$mId} && $HMConfig::culHmModel{$mId}{chn} =~ m/Dim_V/){
		my @chnPh = (grep{$_ =~ m/Dim:/ } split ',',$HMConfig::culHmModel{$mId}{chn});
        @chnPh = split ':',$chnPh[0] if (@chnPh);
        my $chnPhyMax = $chnPh[2]?$chnPh[2] - $chnRemMax:1;         # max Phys channels
        my $chnPhy    = ($chnPhyMax == 2 && $chn >  4 + $chnRemMax)?2:1;    # assotiated phy chan( either 1 or 2)
        my $idPhy     = $src.sprintf("%02X",$chnPhy + $chnRemMax);# ID assot phy chan
        my $pHash     = CUL_HM_id2Hash($idPhy);        # hash assot phy chan
        $idPhy        = $pHash->{DEF};                 # could be device!!!
        if ($pHash){
          $pHash->{helper}{vDim}{idPhy} = $idPhy;
          my $vHash = CUL_HM_id2Hash($src.sprintf("%02X",$chnPhyMax+(2 + $chnRemMax)*$chnPhy-1));
          if ($vHash){
            $pHash->{helper}{vDim}{idV2}  = $vHash->{DEF};
            $vHash->{helper}{vDim}{idPhy} = $idPhy;
          }
          else{
            delete $pHash->{helper}{vDim}{idV2};
          }
          $vHash = CUL_HM_id2Hash($src.sprintf("%02X",$chnPhyMax+(2 + $chnRemMax)*$chnPhy));
          if ($vHash){
            $pHash->{helper}{vDim}{idV3}  = $vHash->{DEF};
            $vHash->{helper}{vDim}{idPhy} = $idPhy;
          }
          else{
            delete $pHash->{helper}{vDim}{idV3};
          }
        }
    }	
	if ($vDim->{idPhy} &&
            CUL_HM_id2Hash($vDim->{idPhy})){   #has virt chan
          RemoveInternalTimer("sUpdt:".$chId);
          if ($mTp eq "10"){               #valid PhysLevel
            foreach my $tmpKey ("idPhy","idV2","idV3",){#update all virtuals
              my $vh = ($vDim->{$tmpKey} ? CUL_HM_id2Hash($vDim->{$tmpKey}) : "");
              next if (!$vh || $vDim->{$tmpKey} eq $chId);
              my $vl = ReadingsVal($vh->{NAME},"level","???");
              my $vs = ($vl eq "100"?"on":($vl eq "0"?"off":"$vl"));
              my($clvlMin,$clvlMax)=split",",AttrVal($vh->{NAME}, "levelRange", "0,100");
              my $plc = int((($pl-$clvlMin)*200)/($clvlMax - $clvlMin))/2;
              $plc = 1 if ($pl && $plc <= 0);
              $vs = ($plc ne $vl)?"chn:$vs  phys:$plc":$vs;
              push @evtEt,[$vh,1,"state:$vs"];
              push @evtEt,[$vh,1,"phyLevel:$pl"];
            }
            push @evtEt,[$shash,1,"phyLevel:$pl"];      #phys level,don't use relative adjustment
            $pl = (($pl-$lvlMin)<=0 && $pl)
                     ? ($pl?1:0)
                     : int((($pl-$lvlMin)*200)/($lvlMax - $lvlMin))/2;
            $physLvl = $pl;
          }
          else{                                #invalid PhysLevel
            $rSUpdt = 1;
            CUL_HM_stateUpdatDly($cName,5) if ($shash->{helper}{dlvl});# update to get level
          }
        }
	my $pVal = $val;# necessary for oper 'off', not logical off
       $val = (($val-$lvlMin)<=0)
                  ? ($val?1:0)
                  : int((($val-$lvlMin)*200)/($lvlMax - $lvlMin))/2;
	
	if(!defined $physLvl){             #not updated? use old or ignore
        $physLvl = ReadingsVal($cName,"phyLevel",$val);
        $physLvl = (($physLvl-$lvlMin)<=0 && $physLvl)
                 ? ($physLvl?1:0)
                 : int((($physLvl-$lvlMin)*200)/($lvlMax - $lvlMin))/2;
      }
	 
	 my $vs = ($shash->{helper}{lm} && $shash->{helper}{lm}{$val})
                     ?$shash->{helper}{lm}{$val}
                     :($val==100 ? "on"
                                 :($pVal==0 ? "off"
                                            : "$val")); # user string...
	 
											
	 my ($timedOn,$stateExt)=("off","");
      if($err&0x40){
        $timedOn = "running";
        $stateExt = "-till" if(AttrVal($cName,"param","") =~ m/showTimed/ );
      }
      push @evtEt,[$shash,1,"level:$val"];
      push @evtEt,[$shash,1,"pct:$val"]; # duplicate to level - necessary for "slider"
      push @evtEt,[$shash,1,"deviceMsg:$vs$target"] if($chId ne "00");
      push @evtEt,[$shash,1,"state:".(($physLvl ne $val)?"chn:$vs phys:$physLvl":$vs.$stateExt)];      
	  push @evtEt,[$shash,1,"overload:".(($err&0x02)?"on":"off")];
      push @evtEt,[$shash,1,"overheat:".(($err&0x04)?"on":"off")];
      push @evtEt,[$shash,1,"reduced:" .(($err&0x08)?"on":"off")];
	  push @evtEt,[$shash,1,"timedOn:".(($err&0x40)?"running":"off")];
	  
    }
}
  # handle sensor event
  elsif($mTp =~ m/^41/ && $p =~ m/^(..)(..)(..)/) {
    my ($chn,$cnt,$val) = (hex($1),hex($2),hex($3)/2);
    my $shash = CUL_HM_id2Hash($src);
	  $chn = sprintf("%02X",$chn & 0x3f);
    # Log 1, "41 ".$model.$chn." ".$val;
    if( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{threeStateSensor} ) {
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$chn};
      Log3 $chnHash->{NAME}, 4, $model.$chn." is threeStateSensor";
      my $vs = ($val==100 ? "open" : ($val==0 ? "closed" : "tilted"));
      push @evtEt,[$chnHash,1,"state:".$vs];
	    push @evtEt,[$chnHash,1,"contact:$vs$target"];
	  }
    elsif( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{ibutton} ) {
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$chn};
      Log3 $chnHash->{NAME}, 4, $model.$chn." is threeStateSensor";
      my $vs = ($val==100 ? "educate" : ($val==0 ? "absent" : "present"));
      push @evtEt,[$chnHash,1,"state:".$vs];
      push @evtEt,[$chnHash,1,"contact:$vs$target"];
    }
    elsif( $HMConfig::culHmRegChan{$model.$chn} == $HMConfig::culHmRegType{motionDetector} && $p =~ m/^(..)(..)(..)(..)/) {
	    my ($bright,$next) = (hex($3),hex($4));
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$chn};
      Log3 $chnHash->{NAME}, 4, $model.$chn." is motionDetector";
	    if( $next ) {
	      my $stamp =  gettimeofday(); # take reception time;
	      $next = (15 << ($next >> 4) - 4); # strange mapping of literals
        RemoveInternalTimer($chnHash->{NAME}.":motionCheck");
        InternalTimer($stamp+$next+2,"CUL_HM_motionCheck", $chnHash->{NAME}.":motionCheck", 0);
        ${chnHash}->{helper}{moStart} = $stamp if (!defined ${chnHash}->{helper}{moStart});
	    }
	    else {
	      $next = "none";
	    }
	    push @evtEt,[$chnHash,1,"state:motion"];
      push @evtEt,[$chnHash,1,"motion:on$target"];
      push @evtEt,[$chnHash,1,"motionCount:$cnt"."_next:$next"."s"];
      push @evtEt,[$chnHash,1,"brightness:$bright"];
	  }
  }
  # handle remote event
  elsif($mTp =~ m/^40/ && $p =~ m/^(..)(..)/) {
    my $shash = CUL_HM_id2Hash($src);
    my ($chn, $bno) = (hex($1), hex($2));# button number/event count
    my $buttonID = sprintf("%02X",$chn&0x3f);# only 6 bit are valid
    # update state of a remote  
    if( $HMConfig::culHmRegChan{$model.$buttonID} == $HMConfig::culHmRegType{remote} ||
        $HMConfig::culHmRegChan{$model.$buttonID} == $HMConfig::culHmRegType{ibutton} ) {
      my $btnName;
      my $state = "";
      my $chnHash = $modules{CUL_HM}{defptr}{$src.$buttonID};
      Log3 $chnHash->{NAME}, 4, $model.$buttonID." is remote";
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
	
# Log(1, "Values Message for Model: ".$model);
# Log(1, "Values Message for Channel: ".$chnnum);
	  if( $model eq "HB-UNI-Sen-LEV-US" ) {
      my $chnHash = $modules{CUL_HM}{defptr}{$src."01"};
      Log3 $chnHash->{NAME}, 4, $model.": ".$values;
      # extract 4 byte value
      my @unpacked = map{hex($_)} unpack("A8",$values);
      push @evtEt,[$chnHash,1,"batVoltage:".$numval/10];
      push @evtEt,[$chnHash,1,"liter:".$unpacked[0]];
      push @evtEt,[$chnHash,1,"level:".$chn." %"];
      push @evtEt,[$chnHash,1,"state:".$chn." %"];
	  }
    #	2018.04.15 21:56:46 1: HB-GEN-SENS01 has 4 values (090400E82803EB0000)
    elsif( $HMConfig::culHmRegChan{$model.$chnnum} == $HMConfig::culHmRegType{values} ) {
	    my $chnHash = $modules{CUL_HM}{defptr}{$src.$chnnum};
      Log3 $chnHash->{NAME}, 4, $model.$chnnum." has $numval values ($p)";
      my $vfmt = AttrVal($chnHash->{NAME},"valuesformat","");
	    # Log 1,$vfmt;
	    if( $vfmt eq "" ) {
	      Log3 $chnHash->{NAME}, 1, "Missing attribute valuesformat at ".$chnHash->{NAME};
		    for( my $i=0; $i<$numval; ++$i ) {
  		    $vfmt = $vfmt."1 ";
		    }
	    }
	    my @valuesfmt = parseValueFormat($vfmt);
	    if( $numval != scalar(@valuesfmt) ) {
	      Log3 $chnHash->{NAME}, 1, "Attribute valuesformat mismatch at $chnHash->{NAME} - expected $numval items but got ".scalar(@valuesfmt)." items";
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
