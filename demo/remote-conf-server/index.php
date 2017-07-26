<?php
header('Content-Type: application/json');
$list = array(
"known_clients" => array(
    array(
        "client_id" => "MuflihunLabs",
        "public_key" => "keys/MuflihunLabs.pub"
    )
),
"known_loggers" => array(
    )
);


echo json_encode($list);
