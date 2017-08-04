<?php

// Usage: php create_request-token.php connection.json

$residue_encrypt = "ripe -e";
$connection = json_decode(file_get_contents($argv[1]));

$request = json_encode(array(
    "logger_id" => "muflihun",
    "token" => "123456789"
));

echo shell_exec("echo '$request' | $residue_encrypt --key $connection->key --client-id $connection->client_id");
