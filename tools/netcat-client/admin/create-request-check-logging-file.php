<?php

$serverKey = file_get_contents("serverkey");
$request = json_encode(array(
    "_t" => round(microtime(true)),
    "type" => 5, // CHECK_LOGGING_FILE
    "logger_id" => "sample-app",
    "client_id" => "muflihun00102030",
    "logging_levels" => ["info", "error", "debug", "warning", "trace", "fatal", "verbose"],
));

echo shell_exec("echo '$request' | ripe -e --key $serverKey");
