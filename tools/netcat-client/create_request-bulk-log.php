<?php

// Usage: php create_request-log.php connection.json

$connection = json_decode(file_get_contents($argv[1]));
function f($c) {
 return array(
        "_t" => time(),
        "datetime" => round(microtime(true) * 1000),
        "logger" => "sample-app",
        "msg" => $c . " Efficient real-time centralized logging server ⚡ ",
        "file" => "index.html",
        "line" => 857,
        "app" => "Muflihun.com",
        "level" => 128
 );
};
$arr = array();
for ($i = 1; $i <= 50; ++$i) {
    $arr[] = f($i);
}
$req = addslashes(json_encode($arr));

echo shell_exec("echo $req | ripe -e --key $connection->key --client-id $connection->client_id");
