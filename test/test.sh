#!/bin/bash

T='gYw'   # The test text

echo -e "\n                 40m     41m     42m     43m\
         44m     45m     46m     47m" > /tmp/colortest;


for FGs in '    m' '   1m' '   2m' '   3m' '   4m' '   5m' '   9m'  \
           '  30m' '1;30m' '2;30m' '3;30m' '4;30m' '5;30m' '9;30m' \
           '  31m' '1;31m' '2;31m' '3;31m' '4;31m' '5;31m' '9;31m' \
           '  32m' '1;32m' '2;32m' '3;32m' '4;32m' '5;32m' '9;32m' \
           '  33m' '1;33m' '2;33m' '3;33m' '4;33m' '5;33m' '9;33m' \
           '  34m' '1;34m' '2;34m' '3;34m' '4;34m' '5;34m' '9;34m' \
           '  35m' '1;35m' '2;35m' '3;35m' '4;35m' '5;35m' '9;35m' \
           '  36m' '1;36m' '2;36m' '3;36m' '4;36m' '5;36m' '9;36m' \
           '  37m' '1;37m' '2;37m' '3;37m' '4;37m' '5;37m' '9;37m' 
do FG=${FGs// /}
    echo -en " $FGs \033[$FG  $T  " >> /tmp/colortest
    for BG in 40m 41m 42m 43m 44m 45m 46m 47m;
    do 
        echo -en "$EINS \033[$FG\033[$BG  $T  \033[0m" >> /tmp/colortest;
    done
    echo -en "\n">> /tmp/colortest;
done
../out2html -i /tmp/colortest
