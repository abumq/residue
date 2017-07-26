<?php

$serverKey = file_get_contents("serverkey");
$request = json_encode(array(
    "_t" => round(microtime(true)),
    "type" => 3, // REMOVE_CLIENT
    "client_id" => "test-client"
));

echo shell_exec("echo '$request' | ripe -e --key $serverKey");
