 php create_request-ping.php connection.json | nc `cat host` 8777 | php decrypt-response.php connection.json > connection_.json && mv connection_.json connection.json
