#!/bin/bash

echo "Residue dispatch error $1. File: $2" | mail -s 'residue err' $3
