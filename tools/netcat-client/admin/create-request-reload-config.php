<?php

$serverKey = file_get_contents("serverkey");
$request = json_encode(array(
    "_t" => round(microtime(true)),
    "type" => 1, // RELOAD_CONFIG
    "logger_id" => "sample-app",
));

echo shell_exec("echo '$request' | ripe -e --key $serverKey");
//printf("echo '$request' | ripe -e --key $serverKey");
