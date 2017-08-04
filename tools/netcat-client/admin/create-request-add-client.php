<?php

$serverKey = file_get_contents("serverkey");
$request = json_encode(array(
    "_t" => round(microtime(true)),
    "type" => 2, // ADD_CLIENT
    "client_id" => "test-client",
    "rsa_public_key" => "/Users/majid.khan/Projects/residue/samples/clients/netcat/client-256-public.pem"
));

echo shell_exec("echo '$request' | ripe -e --key $serverKey");
// Following will fail as we do not allow admin requests using server public key
//echo shell_exec("echo '$request' | ripe -e --in-key server-1024-public.pem") . "\r\n\r\n";
