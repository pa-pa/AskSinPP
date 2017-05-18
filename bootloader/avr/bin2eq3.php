#! /usr/bin/php -q
<?php

define('LF', "\n");

if ($argc < 3 || $argc > 4 || ($argc == 4 && !is_numeric($argv[3])) || in_array($argv[1], array('--help', '-help', '-h', '-?'))) {
	echo 'Helper for converting binary firmware files to files which can flash via Over The Air Update.' . LF . LF;
	echo 'Usage: ./bin2eq3.php <infile> <outfile> [pagesize]' . LF . LF;
	echo '   <infile>:   The filename of the bin file containing the firmware to convert' . LF;
	echo '   <outfile>:  The filename width converted firmware data' . LF;
	echo '   [pagesize]: Number of bytes of the flash page size of the desired MCU (default 256 bytes)' . LF . LF;
	die;
}

$inFile  = $argv[1];
$outFile = $argv[2];

if (!file_exists($inFile)) {
	echo 'Can \'t open infile (' . $inFile . ')' . LF;
	die;
}

$spmPagesize = ($argc == 4) ? (int)$argv[3] : 256;

$fhInfile = fopen($inFile, 'r');

$out = '';
$numPages=0;
while(!feof($fhInfile)) {
	$payload = fread($fhInfile, $spmPagesize);
	if(strlen($payload)==0) continue;
	$numPages++;
	$out .= sprintf('%04X', $spmPagesize);
	for($i = 0; $i < $spmPagesize; $i++) {
		if ($i >= strlen($payload)) {
			$out.= '00';
		} else {
			$out.= sprintf('%02X', ord($payload[$i]));
		}
	}
}

file_put_contents($outFile, $out);

echo 'File "' . $outFile . '" was written with page size of ' . $spmPagesize . ' Bytes and a total of ' . $numPages . ' Pages.' . LF;

