<?php

// Usage: php create_request-log.php connection.json

$connection = json_decode(file_get_contents($argv[1]));

$req = array(
        "_t" => time(),
	"datetime" => round(microtime(true) * 1000),
	"logger" => "sample-app",
	"msg" => "Efficient real-time centralized logging server ⚡ ",
	"file" => "index.html",
	"line" => 857,
	"app" => "Residue Netcat Sample",
	"level" => 4
);
$req = addslashes(json_encode($req));
echo shell_exec("echo $req | ripe -e --key $connection->key --client-id $connection->client_id");
