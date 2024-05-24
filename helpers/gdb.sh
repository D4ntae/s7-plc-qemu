#!/bin/bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)"
VENV_PATH="$ROOT_DIR/python-plugin/venv"

source "$VENV_PATH/bin/activate"

export PYTHONPATH="$VENV_PATH/lib/python3.12/site-packages:$PYTHONPATH"
echo $PYTHONPATH

# Start GDB with any passed arguments
exec gdb-multiarch "$@"
