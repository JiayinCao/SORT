#!/bin/zsh
 
arch_name="$(uname -m)"
 
if [ "${arch_name}" = "x86_64" ]; then
    if [ "$(sysctl -in sysctl.proc_translated)" = "1" ]; then
        echo "Apple Silicon"
    else
        echo "x86_64"
    fi 
elif [ "${arch_name}" = "arm64" ]; then
    echo "Apple Silicon"
else
    echo "Unknown"
fi
