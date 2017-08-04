<?php

// File used by decrypt-respose.php and decrypt-request.php files
global $DATA_INDEX;
$connection = json_decode(file_get_contents($argv[1]));

$f = fopen("php://stdin", "r");

$input = "";
while( $line = fgets( $f ) ) {
  $input .= $line . "\n";
}

fclose( $f );

$parts = explode(":", $input);
if (count($parts) == 1) {
    echo "PLAIN: ";
    echo $parts[0];
    die();
}
$iv = trim($parts[0]);
$data = $parts[$DATA_INDEX];
file_put_contents("/tmp/re_$DATA_INDEX.enc", base64_decode($data));

echo shell_exec("cat /tmp/re_$DATA_INDEX.enc | ripe -d --key $connection->key --iv $iv --clean");
