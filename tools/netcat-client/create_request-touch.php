<?php

$connection = json_decode(file_get_contents($argv[1]));

$request = json_encode(array(
    "_t" => time(),
    "client_id" => "$connection->client_id",
    "type" => 3
));

echo shell_exec("echo '$request' | ripe -e --key $connection->key --client-id $connection->client_id");
