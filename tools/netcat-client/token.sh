php create_request-token.php connection.json | nc `cat host` 8778 | php decrypt-response.php connection.json > tokens/sample-app
