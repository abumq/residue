<?php

$serverKey = file_get_contents("serverkey");
$request = json_encode(array(
    "_t" => round(microtime(true)),
    "type" => 6, // FORCE_LOG_ROTATION
    "logger_id" => "sample-app"
));

echo shell_exec("echo '$request' | ripe -e --key $serverKey");
