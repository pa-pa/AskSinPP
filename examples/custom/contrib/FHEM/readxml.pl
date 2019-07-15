
#
# extract parameter from CCU-XML and convert into FHEM format
# This will not handle all possibile settings, but generates the base structure
#

use strict;
use warnings;

use XML::LibXML;
use Data::Dumper qw(Dumper);


sub convName ($) {
  my $name = lc $_[0];
  my $result = "";
  my $nextupper = 0;
  while( $name =~ /(.)/g ) {
    if( $1 ne "_" ) {
      my $c = $1;
      $c = uc $1 if $nextupper;
      $result = $result.$c;
      $nextupper = 0;
    }
    else {
      $nextupper = 1;  
    }
  }
  return $result;
}

my $dom = XML::LibXML->load_xml(location => $ARGV[0]);

#print Dumper $dom;

foreach my $param ($dom->findnodes('//paramset/parameter')) {
  my $logical = ($param->findnodes('./logical'))[0];
  my $physical = ($param->findnodes('./physical'))[0];
  my $conv = ($param->findnodes('./conversion'))[0];
  if( defined $physical && defined $logical && $physical->getAttribute('interface') eq "config" ) {
    my $list = $physical->getAttribute('list');
    my $name = convName($param->getAttribute('id'));
    print '$HMConfig::culHmRegDefine{"'.$name.'"} = {';
    print "a=>".$physical->getAttribute('index');
    print ",s=>".$physical->getAttribute('size');
    print ",l=>".$list;
    my $min = $logical->getAttribute('min');
    my $max = $logical->getAttribute('max');
    if( ! defined $min && ! defined $max ) {
      if( $logical->getAttribute('type') eq "boolean" ) {
        $min=0;
        $max=1;
      }
      else {
        $min = $max = 0;  
      }
    }
    print ",min=>".$min.",max=>".$max;
    print ",c=>''";
    print ",p=>'".($list == 3 || $list == 4 ? "y" : "n")."'";
    my $factor = $conv->getAttribute('factor') if defined $conv;
    print ",f=>".(defined $factor ? $factor : "''");
    my $unit = $logical->getAttribute('unit');
    print ",u=>'".(defined $unit ? $unit : "")."'";
    print ",d=>1,t=>\"".$name."\"";
    my @special = $logical->findnodes('./special_value');
    if( @special ) {
      print ",lit=>{";
      foreach my $l (@special) {
        print convName($l->getAttribute('id')).'=>'.$l->getAttribute('value').",";
      }
      print "}";
    }
    print "}\n";
  }
}