# Config Validator
You can use this tool to validate residue configuration file. Exit value is `1` if invalid otherwise `0`.

### Synopsis
```
	config-validator --version
	config-validator --in <file>
	echo <json> | config-validator
```

### Options

| Option Name | Description |
|-------------|--------|
| `--version` | Display version information
| `--in`    | Input configuration file |
| `--no-json`    | If invalid configuration file because of malformed JSON it will ignore the actual JSON in the output |