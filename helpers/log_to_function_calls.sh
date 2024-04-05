#!/usr/bin/env bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)"

cat $ROOT_DIR/log.txt | grep b | awk '{if($4 ~ /^#/) print substr($1, 1, length($1) - 1), $4}' | awk '/0x000400./{flag=1} flag' | tee $ROOT_DIR/function_calls.log
