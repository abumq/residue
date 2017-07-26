<?php

$f = fopen("php://stdin", "r");

$input = "";
while( $line = fgets( $f ) ) {
  $input .= $line . "\n";
}

fclose( $f );

$parts = explode(":", $input);

$offset = 0;
$iv = $parts[$offset];

file_put_contents("connect.base64.enc", $parts[$offset+1]); 
file_put_contents("connect.enc", base64_decode($parts[$offset+1])); 
file_put_contents("iv.key", $iv); 
