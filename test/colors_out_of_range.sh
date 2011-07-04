#!/bin/bash

for i in `seq 0 9`; do
	echo -en "\e[3${i}mcolor 3${i}\n" | ../out2html 
done

exit 0;
