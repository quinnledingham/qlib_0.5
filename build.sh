#!/bin/bash

code="$PWD"
opts=-g
cd Q:\build\ > /dev/null
g++ $opts $code/Q:\build\ -o Q:\build\
cd $code > /dev/null
