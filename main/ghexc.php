<?php
/* Generate hex content from file so can be used as c variable. 
 * 
 * Usage: php ghexc.php filenameToHex.html 
 * 
 * */

//echo "count argv: ".count($argv)."\n";
//
if( count($argv)<2 ) {
	die("Usage: php ghexc.php filenameToHex.html\n\n");
}

$fn   = $argv[1];
$mc   = 30;
$skip = [];//["\xa",];

//
$tmp = file_get_contents( $fn );
//
echo "filesize: ".strlen($tmp)."\n\n";
//
$data = "";
$cnt  = 0;
$len  = strlen($tmp);
//
for($i=0; $i<$len; $i++) {
	if( in_array($tmp[$i],$skip) ) {
		$len--;
		continue;
    }
    $c = dechex(ord($tmp[$i]));
    $data .= ($i==0?"char yourhtml[] = \"":($cnt==0?"\"":""))."\\x".$c.($i>=($len-1)?"\";\n":($cnt>=$mc?"\"\n":""));
    if($cnt>=$mc)$cnt=0;
    else $cnt++;
}

echo "DONE( ".strlen($data)." ): \n\n".$data."\n\n";
?>
