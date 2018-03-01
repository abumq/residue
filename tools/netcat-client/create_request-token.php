<?php

// Usage: php create_request-token.php connection.json

$connection = json_decode(file_get_contents($argv[1]));

$request = json_encode(array(
    "_t" => time(),
    "logger_id" => "sample-app",
    "access_code" => "a2dcb"
));

echo shell_exec("echo '$request' | ripe -e --key $connection->key --client-id $connection->client_id");
