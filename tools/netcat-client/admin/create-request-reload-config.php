<?php

$serverKey = file_get_contents("serverkey");
$request = json_encode(array(
    "_t" => round(microtime(true)),
    "type" => 1, // RELOAD_CONFIG
));

echo shell_exec("echo '$request' | ripe -e --key $serverKey");
// Following will fail as we do not allow admin requests using server public key
//echo shell_exec("echo '$request' | ripe -e --in-key server-1024-public.pem") . "\r\n\r\n";
