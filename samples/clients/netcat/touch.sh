 php create_request-touch.php connection.json | nc `cat host` 8777 | php decrypt-response.php connection.json > connection_.json && mv connection_.json connection.json
