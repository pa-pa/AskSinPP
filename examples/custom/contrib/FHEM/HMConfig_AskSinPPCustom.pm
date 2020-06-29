package main;

use strict;
use warnings;
use HMMsg;

############globals############
use vars qw(%customMsg);

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

# HB-UNI-Sen-PRESS
$HMConfig::culHmRegDefine{"sendIntervalPress"}   = {a=>33, s=>2,l=>0,min=>20,max=>600, c=>'',   p=>'n',f=>'',u=>'s', d=>1,t=>"send interval"};
$HMConfig::culHmRegDefine{"sensorTypePress"}     = {a=>39 ,s=>1,l=>1,min=>0,max=>1,    c=>'lit',p=>'n',f=>'',u=>'',  d=>1,t=>"sensor type",lit=>{type12MPa=>0,type05MPa=>1}};

# HB-UNI-Sen-CAP-MOIST-T
$HMConfig::culHmRegDefine{"tempOffset"}         = {a=>1,  s=>4,l=>1,min=>-5,max=>5,   c=>'',   p=>'n',f=>10,u=>'°C', d=>1,t=>"temperature offset"};
$HMConfig::culHmRegDefine{"transmitInterval"}   = {a=>33, s=>2,l=>0,min=>1,max=>1440, c=>'',   p=>'n',f=>'',u=>'min',  d=>1,t=>"transmit interval"};
$HMConfig::culHmRegDefine{"highValue"}          = {a=>35, s=>2,l=>1,min=>0,max=>1023, c=>'',   p=>'n',f=>'',u=>'',     d=>1,t=>"high value"};
$HMConfig::culHmRegDefine{"lowValue"}           = {a=>37, s=>2,l=>1,min=>0,max=>1023, c=>'',   p=>'n',f=>'',u=>'',     d=>1,t=>"low value"};
$HMConfig::culHmRegDefine{"lowBatteryLimit"}    = {a=>18 ,s=>1,l=>0,min=>0,max=>15   ,c=>'',   p=>'n',f=>10,u=>'V',    d=>1,t=>"low batterie limit, step .1V"};

# HB-UNI-Sen-WEA
$HMConfig::culHmRegDefine{"altitude"}         = {a=>34,  s=>2,l=>0,min=>0,max=>10000, c=>'',   p=>'n',f=>'',u=>'m', d=>1,t=>"altitude"};

$HMConfig::culHmRegDefine{"anemometerRadius"}            = {a=>1.0, s=>1,l=>1,min=>0,max=>25.5,c=>'',   p=>'n',f=>10,  u=>'cm',  d=>1,t=>"Anemometer Radius"};
$HMConfig::culHmRegDefine{"anemometerCalibrationFactor"} = {a=>2.0, s=>2,l=>1,min=>0,max=>1000,c=>'',   p=>'n',f=>10,  u=>'',    d=>1,t=>"Anemometer Calibration Factor"};
$HMConfig::culHmRegDefine{"ldCapacitor"}                 = {a=>4.0, s=>1,l=>1,min=>0,max=>120, c=>'',   p=>'n',f=>'',  u=>'pF',  d=>1,t=>"Light Detector Capacitor"};
$HMConfig::culHmRegDefine{"ldDisturberDetection"}        = {a=>5.0, s=>1,l=>1,min=>0,max=>1,   c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Light Detector Disturber Detection"};
$HMConfig::culHmRegDefine{"ldMinstrikes"}                = {a=>18.0,s=>1,l=>1,min=>0,max=>3,   c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Light Detector Minstrikes"};
$HMConfig::culHmRegDefine{"ldWdthreshold"}               = {a=>19.0,s=>1,l=>1,min=>0,max=>3,   c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Light Detector Wdthreshold"};
$HMConfig::culHmRegDefine{"ldNoisefloorlevel"}           = {a=>20.0,s=>1,l=>1,min=>0,max=>7,   c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Light Detector Noisefloorlevel"};
$HMConfig::culHmRegDefine{"ldSpikerejection"}            = {a=>21.0,s=>1,l=>1,min=>0,max=>7,   c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Light Detector Spikerejection"};
$HMConfig::culHmRegDefine{"gustThresholdExtraMessage"}   = {a=>6.0, s=>1,l=>1,min=>0,max=>255, c=>'',   p=>'n',f=>'',  u=>'km/h',d=>1,t=>"Gust Threshold Extra Message"};
$HMConfig::culHmRegDefine{"rdSensorType"}                = {a=>17.0,s=>1,l=>1,min=>0,max=>0,   c=>'lit',p=>'n',f=>'',  u=>'',    d=>1,t=>"Rain Detector Sensor Type",lit=>{generic=>0,stallbiz=>1}};
$HMConfig::culHmRegDefine{"rdStallbizHiThresholdRain"}   = {a=>9.0, s=>2,l=>1,min=>0,max=>1023,c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Rain Detector Stallbiz Hi Threshold Rain"};
$HMConfig::culHmRegDefine{"rdStallbizLoThresholdRain"}   = {a=>11.0,s=>2,l=>1,min=>0,max=>1023,c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Rain Detector Stallbiz Threshold Rain"};
$HMConfig::culHmRegDefine{"rdStallbizHiThresholdHeater"} = {a=>13.0,s=>2,l=>1,min=>0,max=>1023,c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Rain Detector Stallbiz Hi Threshold Heater"};
$HMConfig::culHmRegDefine{"rdStallbizLoThresholdHeater"} = {a=>15.0,s=>2,l=>1,min=>0,max=>1023,c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Rain Detector Stallbiz Lo Threshold Heater"};
$HMConfig::culHmRegDefine{"rdStallbizHeatOnDewfall"}     = {a=>22.0,s=>1,l=>1,min=>0,max=>1,   c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Rain Detector Stallbiz Heat On Dewfall"};
$HMConfig::culHmRegDefine{"stormUpperThreshold"}         = {a=>7.0, s=>1,l=>1,min=>0,max=>0xc8,c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Storm Upper Threshold"};
$HMConfig::culHmRegDefine{"stormLowerThreshold"}         = {a=>8.0, s=>1,l=>1,min=>0,max=>0xc8,c=>'',   p=>'n',f=>'',  u=>'',    d=>1,t=>"Storm Lower Threshold"};


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
$customMsg{"HM-LC-Sw2-FM-CustomFW"} = sub {
  my ($msg,$target) = @_;
  return $msg->processRemote if $msg->isRemote;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};

$HMConfig::culHmModel{"F336"} = {name=>"HM-LC-Sw2PBU-FM-CustomFW",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p,4:3p.4p',chn=>"Sw:1:2,Btn:3:4"};
$HMConfig::culHmChanSets{"HM-LC-Sw2PBU-FM-CustomFW00"}{fwUpdate} ="<filename>";
$HMConfig::culHmChanSets{"HM-LC-Sw2PBU-FM-CustomFW01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HM-LC-Sw2PBU-FM-CustomFW02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HM-LC-Sw2PBU-FM-CustomFW03"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw2PBU-FM-CustomFW04"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegChan{"HM-LC-Sw2PBU-FM-CustomFW01"}  = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan{"HM-LC-Sw2PBU-FM-CustomFW02"}  = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan{"HM-LC-Sw2PBU-FM-CustomFW03"}  = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan{"HM-LC-Sw2PBU-FM-CustomFW04"}  = $HMConfig::culHmRegType{remote};
$customMsg{"HM-LC-Sw2PBU-FM-CustomFW"} = sub {
  my ($msg,$target) = @_;
  return $msg->processRemote if $msg->isRemote;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};

$HMConfig::culHmModel{"F332"} = {name=>"HB-UNI-SenAct-4-4-RC",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p.3p.4p,4:5p.6p.7p.8p',chn=>"Sw:1:4,Btn:5:8"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC04"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC05"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC06"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC07"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-4-4-RC08"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC01"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC03"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC04"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC05"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC06"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC07"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-4-4-RC08"} = $HMConfig::culHmRegType{remote};
$customMsg{"HB-UNI-SenAct-4-4-RC"} = sub {
  my ($msg,$target) = @_;
  return $msg->processRemote if $msg->isRemote;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};

$HMConfig::culHmModel{"F335"} = {name=>"HM-LC-Sw1-FM-Cus",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:3p,3:1p.2p',chn=>"Btn:1:2,Sw:3:3"};
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus02"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HM-LC-Sw1-FM-Cus03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmRegChan {"HM-LC-Sw1-FM-Cus01"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HM-LC-Sw1-FM-Cus02"} = $HMConfig::culHmRegType{remote};
$HMConfig::culHmRegChan {"HM-LC-Sw1-FM-Cus03"} = $HMConfig::culHmRegType{switch};
$customMsg{"HM-LC-Sw1-FM-Cus"} = sub {
  my ($msg,$target) = @_;
  return $msg->processRemote if $msg->isRemote;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};

$HMConfig::culHmModel{"F202"} = {name=>"HB-SW2-SENS",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p,4:3p',chn=>"Sw:1:2,Sen:3:3"};
$HMConfig::culHmChanSets{"HB-SW2-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-SW2-SENS01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-SW2-SENS03"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegChan {"HB-SW2-SENS01"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-SW2-SENS02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-SW2-SENS03"} = $HMConfig::culHmRegType{threeStateSensor};
$customMsg{"HB-SW2-SENS"} = sub {
  my ($msg,$target) = @_;
  my $channel = $msg->channel;
  return $msg->processThreeState($target) if $channel == 3;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};

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
$HMConfig::culHmRegModel{"HB-DoorBell"}   = { backOnTime=>1 };
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
$customMsg{"HB-DoorBell"} = sub {
  my ($msg,$target) = @_;
  my $channel = $msg->channel;
  return $msg->processSwitchStatus($target) if $msg->isStatus && ($channel == 2 || $channel == 3);
  return $msg->processRemote if $msg->isRemote;
  return $msg->processValues if $msg->isValues;
  return $msg->processMotion($target) if $channel == 5;
  return $msg->processThreeState($target,(0=>'absent',200=>'present',100=>'educate')) if $channel >= 6;
  return ();
};

$HMConfig::culHmModel{"F205"} = {name=>"HB-GEN-SENS",st=>'custom',cyc=>'',rxt=>'c',lst=>'1',chn=>"Values:1:1"};
$HMConfig::culHmChanSets{"HB-GEN-SENS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-GEN-SENS01"} = $HMConfig::culHmSubTypeSets{"Values"};
$HMConfig::culHmRegChan {"HB-GEN-SENS01"} = $HMConfig::culHmRegType{values};
$customMsg{"HB-GEN-SENS"} = sub {
  my ($msg,$target) = @_;
  return $msg->processValues if $msg->isValues;
  return ();
};

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
$HMConfig::culHmRegModel{"HB-IBUT-8"}   = { buttonMode=>1 };
$HMConfig::culHmRegChan {"HB-IBUT-801"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-802"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-803"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-804"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-805"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-806"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-807"} = $HMConfig::culHmRegType{ibutton};
$HMConfig::culHmRegChan {"HB-IBUT-808"} = $HMConfig::culHmRegType{ibutton};
$customMsg{"HB-IBUT-8"} = sub {
  my ($msg,$target) = @_;
  return $msg->processRemote if $msg->isRemote;
  return $msg->processThreeState($target,(0=>'absent',200=>'present',100=>'educate'));
  return ();
};

# 2 channel blind actor
$HMConfig::culHmModel{"F207"} = {name=>"HB-LC-BL1-FM-2",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p',chn=>"Blind:1:2"};
$HMConfig::culHmChanSets{"HB-LC-BL1-FM-200"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-LC-BL1-FM-201"} = $HMConfig::culHmSubTypeSets{"blindActuator"};
$HMConfig::culHmChanSets{"HB-LC-BL1-FM-202"} = $HMConfig::culHmSubTypeSets{"blindActuator"};
#$HMConfig::culHmRegModel{"HB-LC-BL1-FM-2"}   = {};
$HMConfig::culHmRegChan {"HB-LC-BL1-FM-201"} = $HMConfig::culHmRegType{blindActuator};
$HMConfig::culHmRegChan {"HB-LC-BL1-FM-202"} = $HMConfig::culHmRegType{blindActuator};
$customMsg{"HB-LC-BL1-FM-2"} = sub {
  my ($msg,$target) = @_;
  return $msg->processBlindStatus($target) if $msg->isStatus;
  return ();
};

# 4 channel switch actor with extra motion sensor
$HMConfig::culHmModel{"F208"} = {name=>"HB-LC-SW4-MDIR",st=>'custom',cyc=>'',rxt=>'',lst=>'1,3:1p.2p.3p.4p,4:5p',chn=>"Sw:1:4,Motion:5:5"};
$HMConfig::culHmChanSets{"HB-LC-SW4-MDIR00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-LC-SW4-MDIR01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-LC-SW4-MDIR02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-LC-SW4-MDIR03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-LC-SW4-MDIR04"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-LC-SW4-MDIR05"} = $HMConfig::culHmSubTypeSets{"THSensor"};
#$HMConfig::culHmRegModel{"HB-LC-SW4-MDIR"}   = {};
$HMConfig::culHmRegChan {"HB-LC-SW4-MDIR01"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-LC-SW4-MDIR02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-LC-SW4-MDIR03"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-LC-SW4-MDIR04"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-LC-SW4-MDIR05"} = $HMConfig::culHmRegType{motionDetector};
$customMsg{"HB-LC-SW4-MDIR"} = sub {
  my ($msg,$target) = @_;
  my $channel = $msg->channel;
  return $msg->processMotion($target) if $channel == 5;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};

# window contact RHS3
$HMConfig::culHmModel{"F209"} = {name=>"HB-Sec-RHS-3",st=>'custom',cyc=>'28:00',rxt=>'c:l',lst=>'1,4:1p',chn=>""};
$HMConfig::culHmChanSets{"HB-Sec-RHS-300"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegModel{"HB-Sec-RHS-3"}   = { lowBatLimitBA2=>1, sabotageMsg=>1, transmDevTryMax=>1, cyclicInfoMsg=>1,
                                               msgRhsPosA=>1, msgRhsPosB=>1, msgRhsPosC=>1, ledOnTime=>1, eventDlyTime=>1 };
$customMsg{"HB-Sec-RHS-3"} = sub {
  my ($msg,$target) = @_;
  my $batflags = 0;
  my $bat = 0;
  my $device = main::CUL_HM_id2Hash($msg->from);
  my @evtEt = $msg->processThreeState($target,(0=>'closed',50=>'unlocked',100=>'tilted',200=>'open')) if $msg->channel == 1;
  if( $msg->isSensor ) {
    # add battery value
    $bat = $msg->payloadByte(3);
    $batflags = $msg->payloadByte(0);
  }
  if( $msg->isStatus ) {
    # add battery value
    $bat = $msg->payloadByte(5);
    $batflags = $msg->payloadByte(3);
    # set sabotage status
    push @evtEt,[$device,1,"sabotageError:".(($batflags & 0x0E) ? "on" : "off")];
  }
  # add battery state
  my $batstat = "ok";
  $batstat = "low" if (($batflags & 0x80)==0x80);
  push @evtEt,[$device,1,"battery:".$batstat];
  push @evtEt,[$device,1,"batVoltage:".$bat/10];
  return @evtEt;
};

# velux blind - simple blind with burst and battery state
$HMConfig::culHmModel{"F20A"} = {name=>"HB-LC-Bl1-Velux",st=>'custom',cyc=>'',rxt=>'b',lst=>'1,3',chn=>""};
$HMConfig::culHmChanSets{"HB-LC-Bl1-Velux00"} = $HMConfig::culHmSubTypeSets{"blindActuator"};
$HMConfig::culHmRegModel{"HB-LC-Bl1-Velux"} = $HMConfig::culHmRegType{blindActuator};
$customMsg{"HB-LC-Bl1-Velux"} = sub {
  my ($msg,$target) = @_;
  my $device = main::CUL_HM_id2Hash($msg->from);
  if( $msg->isStatus ) {
    my @evtEt = $msg->processBlindStatus($target,$device);
    my $batflags = $msg->payloadByte(3);
    my $batstat = "ok";
    $batstat = "low" if (($batflags & 0x80)==0x80);
    push @evtEt,[$device,1,"battery:".$batstat];
    return @evtEt;
  }
  return ();
};

$HMConfig::culHmModel{"F9D2"} = {name=>"HB-UNI-Sen-LEV-US",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1',chn=>"Level:1:1"};
$HMConfig::culHmChanSets{"HB-UNI-Sen-LEV-US00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-Sen-LEV-US01"} = {};
$HMConfig::culHmRegModel{"HB-UNI-Sen-LEV-US"}   = { lowBatteryLimit=>1, sendInterval=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-LEV-US01"} = { distanceOffset=>1, sensorType=>1, caseDesign=>1, caseHeight=>1, caseWidth=>1, caseLength=>1 };
$customMsg{"HB-UNI-Sen-LEV-US"} = sub {
  my ($msg,$target) = @_;
  my @evtEt=();
  my $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId(1)};
  if( defined($channel) ) {
    my $percent = $msg->payloadByte(0);
    my $bat = $msg->payloadByte(1)/10;
    my $liter = $msg->payloadLong(2);
    push @evtEt,[$channel,1,"batVoltage:".$bat/10];
    push @evtEt,[$channel,1,"liter:".$liter];
    push @evtEt,[$channel,1,"level:".$percent." %"];
    push @evtEt,[$channel,1,"state:".$percent." %"];
  }
  return @evtEt;
};

$HMConfig::culHmModel{"E901"} = {name=>"HB-UNI-Sen-PRESS",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1',chn=>"Pressure:1:1"};
$HMConfig::culHmChanSets{"HB-UNI-Sen-PRESS00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-Sen-PRESS01"} = {};
$HMConfig::culHmRegModel{"HB-UNI-Sen-PRESS"}   = { sendIntervalPress=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-PRESS01"} = { sensorTypePress=>1 };
$customMsg{"HB-UNI-Sen-PRESS"} = sub {
  my ($msg,$target) = @_;
  my @evtEt=();
  my $cnum = $msg->payloadByte(2); # get channel from byte 2 of payload
  my $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId($cnum)};
  if( defined($channel) ) {
    my $value = $msg->payloadWord(3);
    push @evtEt,[$channel,1,"pressure:".$value/100];
    push @evtEt,[$channel,1,"state:".$value/100];
  }
  return @evtEt;
};

$HMConfig::culHmModel{"F311"} = {name=>"HB-UNI-Sen-CAP-MOIST",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1',chn=>"Data:1:1,Moisture:2:4"};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST01"} = {};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST02"} = {};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST03"} = {};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST04"} = {};
$HMConfig::culHmRegModel{"HB-UNI-Sen-CAP-MOIST"}   = { lowBatteryLimit=>1, transmitInterval=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST01"} = {};
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST02"} = { highValue=>1, lowValue=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST03"} = { highValue=>1, lowValue=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST04"} = { highValue=>1, lowValue=>1 };
$customMsg{"HB-UNI-Sen-CAP-MOIST"} = sub {
  my ($msg,$target) = @_;
  my @evtEt=();
  my $cnum = $msg->payloadByte(1) & 0x3f; # get channel from byte 1 of payload
  my $device = main::CUL_HM_id2Hash($msg->from);
  my $batstat = "ok";
  $batstat = "low" if (($msg->payloadByte(0) & 0x80)==0x80);
  push @evtEt,[$device,1,"battery:".$batstat];
  my $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId($cnum)};
  if( defined($channel) ) {
    my $bat = $msg->payloadByte(2);
    push @evtEt,[$channel,1,"batVoltage:".$bat/10];
    push @evtEt,[$channel,1,"state:".($bat/10)." V"];
  }
  for( my $offset=3; $offset < length($msg->payload)/2; $offset += 2 ) {
    $cnum = $msg->payloadByte($offset) & 0x3f; # get channel for next value
    $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId($cnum)};
    if( defined($channel) ) {
      my $moist = $msg->payloadByte($offset+1);
      push @evtEt,[$channel,1,"humidity:".$moist];
      push @evtEt,[$channel,1,"state:".$moist." %"];
    }
    else {
      Log 1,"No channel for ".$msg->channelId($cnum);
    }
  }
  return @evtEt;
};

$HMConfig::culHmModel{"F312"} = {name=>"HB-UNI-Sen-CAP-MOIST-T",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1',chn=>"Weather:1:1,Moisture:2:4"};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST-T00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST-T01"} = {};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST-T02"} = {};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST-T03"} = {};
$HMConfig::culHmChanSets{"HB-UNI-Sen-CAP-MOIST-T04"} = {};
$HMConfig::culHmRegModel{"HB-UNI-Sen-CAP-MOIST-T"}   = { lowBatteryLimit=>1, transmitInterval=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST-T01"} = { tempOffset=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST-T02"} = { highValue=>1, lowValue=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST-T03"} = { highValue=>1, lowValue=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-CAP-MOIST-T04"} = { highValue=>1, lowValue=>1 };
$customMsg{"HB-UNI-Sen-CAP-MOIST-T"} = sub {
  my ($msg,$target) = @_;
  my @evtEt=();
  my $cnum = $msg->payloadByte(1) & 0x3f; # get channel from byte 1 of payload
  my $device = main::CUL_HM_id2Hash($msg->from);
  my $batstat = "ok";
  $batstat = "low" if (($msg->payloadByte(0) & 0x80)==0x80);
  push @evtEt,[$device,1,"battery:".$batstat];
  my $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId($cnum)};
  if( defined($channel) ) {
    my $temp = $msg->payloadWord(2);
    my $bat = $msg->payloadByte(4);
    push @evtEt,[$channel,1,"batVoltage:".$bat/10];
    push @evtEt,[$channel,1,"temperature:".$temp/10];
    push @evtEt,[$channel,1,"state:".$temp/10];
  }
  for( my $offset=5; $offset < length($msg->payload)/2; $offset += 2 ) {
    $cnum = $msg->payloadByte($offset) & 0x3f; # get channel for next value
    $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId($cnum)};
    if( defined($channel) ) {
      my $moist = $msg->payloadByte($offset+1);
      push @evtEt,[$channel,1,"humidity:".$moist];
      push @evtEt,[$channel,1,"state:".$moist." %"];
    }
    else {
      Log 1,"No channel for ".$msg->channelId($cnum);
    }
  }
  return @evtEt;
};

$HMConfig::culHmModel{"F33A"} = {name=>"HB-UNI-SenAct-8-8-SC",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1,3:1p.2p.3p.4p.5p.6p.7p.8p,4:9p.10p.11p.12p.13p.14p.15p.16p',chn=>"Sw:1:8,Sen:9:16"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC01"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC02"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC03"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC04"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC05"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC06"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC07"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC08"} = $HMConfig::culHmSubTypeSets{"switch"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC09"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC0A"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC0B"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC0C"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC0D"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC0E"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC0F"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmChanSets{"HB-UNI-SenAct-8-8-SC10"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegModel{"HB-UNI-SenAct-8-8-SC"}   = { intKeyVisib=>1, cyclicInfoMsg=>1, sabotageMsg=>1 };
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC01"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC02"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC03"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC04"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC05"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC06"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC07"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC08"} = $HMConfig::culHmRegType{switch};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC09"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC0A"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC0B"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC0C"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC0D"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC0E"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC0F"} = $HMConfig::culHmRegType{threeStateSensor};
$HMConfig::culHmRegChan {"HB-UNI-SenAct-8-8-SC10"} = $HMConfig::culHmRegType{threeStateSensor};
$customMsg{"HB-UNI-SenAct-8-8-SC"} = sub {
  my ($msg,$target) = @_;
  my $channel = $msg->channel;
  return $msg->processThreeState($target) if $channel > 8;
  return $msg->processSwitchStatus($target) if $msg->isStatus;
  return ();
};


$HMConfig::culHmModel{"F1D0"} = {name=>"HB-UNI-Sen-WEA",st=>'custom',cyc=>'',rxt=>'c:l',lst=>'1,4:1p',chn=>"Weather:1:1"};
$HMConfig::culHmChanSets{"HB-UNI-Sen-WEA00"}{fwUpdate} = "<filename>";
$HMConfig::culHmChanSets{"HB-UNI-Sen-WEA01"} = $HMConfig::culHmSubTypeSets{"THSensor"};
$HMConfig::culHmRegModel{"HB-UNI-Sen-WEA"}   = { sendInterval=>1, altitude=>1, transmDevTryMax=>1 };
$HMConfig::culHmRegChan {"HB-UNI-Sen-WEA01"} = { anemometerRadius=>1, anemometerCalibrationFactor=>1, ldCapacitor=>1, ldDisturberDetection=>1,
                                                 ldMinstrikes=>1, ldWdthreshold=>1, ldNoisefloorlevel=>1, ldSpikerejection=>1,
                                                 gustThresholdExtraMessage=>1, rdSensorType=>1, rdStallbizHiThresholdRain=>1,
                                                 rdStallbizLoThresholdRain=>1, rdStallbizHiThresholdHeater=>1, rdStallbizLoThresholdHeater=>1,
                                                 rdStallbizHeatOnDewfall=>1, stormUpperThreshold=>1, stormLowerThreshold=>1,
                                                 peerNeedsBurst=>1, expectAES=>1 };
$customMsg{"HB-UNI-Sen-WEA"} = sub {
  my ($msg,$target) = @_;
  my @evtEt=();
  my $channel = $main::modules{CUL_HM}{defptr}{$msg->channelId(1)}; # fixed channel 1
  if( defined($channel) ) {
    my $temp = $msg->payloadWord(0) & 0x7fff;
    my $pressure = $msg->payloadWord(2);
    my $humidity = $msg->payloadByte(4);
    my $lux = $msg->payload3Byte(5);
    my $raincount = $msg->payloadWord(8);
    my $raining = (($raincount & 0x8000) == 0) ? "off" : "on";
    my $windspeed = ($msg->payloadWord(10) & 0x3fff) / 10;
    my $winddirrange = $msg->payloadWord(10) >> 14;
    my $winddir = $msg->payloadByte(12);
    my $gustspeed = ($msg->payloadWord(13) & 0x3fff) / 10;
    my $rdheating = (($msg->payloadByte(13) & 0x80) == 0) ? "off" : "on";
    my $uvindex = $msg->payloadByte(15) & 0x0f;
    my $lightningdistance = $msg->payloadByte(15) >> 4;
    my $lightningcounter = $msg->payloadByte(16);
    push @evtEt,[$channel,1,"temperature:".$temp/10];
    push @evtEt,[$channel,1,"pressure:".$pressure/10];
    push @evtEt,[$channel,1,"humidity:".$humidity];
    push @evtEt,[$channel,1,"lux:".$lux/10];
    push @evtEt,[$channel,1,"raining:".$raining];
    push @evtEt,[$channel,1,"raincount:".($raincount & 0x7fff)];
    push @evtEt,[$channel,1,"windspeed:".$windspeed/10];
    push @evtEt,[$channel,1,"winddirrange:".($winddirrange*45)/2];
    push @evtEt,[$channel,1,"winddir:".$winddir*3];
    push @evtEt,[$channel,1,"gustspeed:".$gustspeed/10];
    push @evtEt,[$channel,1,"rdheating:".$rdheating];
    push @evtEt,[$channel,1,"uvindex:".$uvindex];
    push @evtEt,[$channel,1,"lightningdistance:".$lightningdistance*3];
    push @evtEt,[$channel,1,"lightningcounter:".$lightningcounter];
    push @evtEt,[$channel,1,"state:T: ".($temp/10)." P: ".($pressure/10)." H: ".$humidity];
  }
  else {
    Log 1,"No channel for ".$msg->channelId(1);
  }
  return @evtEt;
};


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

# parse the incomming messages
sub CUL_HM_Parsecustom($$$$$$) {
  my($mFlg,$mTp,$src,$dst,$p,$target) = @_;
#  Log 1,"General  entering with $mFlg,$mTp,$src,$dst,$p,$target";
#  Log 1, $src;
#  Log 1, CUL_HM_id2Hash($src)->{NAME};
  my $model = AttrVal(CUL_HM_id2Hash($src)->{NAME},"model","");
#  Log 1, $model;
  
  if( defined $customMsg{$model} ) {
    my $msg = HMMsg->new($mTp,$mFlg,$src,$dst,$p);
#    Log 1, "Call custom Message Handler for ".$model;
    return $customMsg{$model}($msg,$target);
  }
  
  my @evtEt = ();
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
  } else {
    Log(1, "HMConfig_AskSinPPCustom received unknown message: $mFlg,$mTp,$src,$dst,$p");
  }
  return @evtEt;
}

1;
