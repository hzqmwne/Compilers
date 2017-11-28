#!/bin/bash

LAB5_PATH=../lab5

if [ $# -eq 1 ]; then
	LAB5_PATH=$1
fi

cp $LAB5_PATH/tiger.lex ./
cp $LAB5_PATH/tiger.y ./

cp $LAB5_PATH/frame.h ./
cp $LAB5_PATH/x86frame.c ./

cp $LAB5_PATH/semant.{c,h} ./
cp $LAB5_PATH/translate.{c,h} ./

# copy other files if necessary
