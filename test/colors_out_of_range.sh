#!/bin/bash

for i in `seq 0 9`; do
	echo -en "\e[3$im\n" | ../ascii2html 2>&1
done
