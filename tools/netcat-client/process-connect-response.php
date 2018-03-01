<?php

$f = fopen("php://stdin", "r");

$json_file = "";
while( $line = fgets( $f ) ) {
  $json_file .= $line . "\n";
}

fclose( $f );

$json = json_decode($json_file);

if ($json->status != 0) {
    echo "ERROR: " . $json->error_text . "\n";
    exit(1);
}

$acknowledgeRequest = json_encode(array(
    "_t" => time(),
    "client_id" => $json->client_id,
    "type" => 2 // Acknowledgement
));

$key = $json->key;
file_put_contents("tmp.key", $key);
echo "Acknowledging...[$key]\n";
shell_exec("echo '$acknowledgeRequest' | ripe -e --key $key --client-id $json->client_id | nc `cat host` 8777 | php save-connection.php");

