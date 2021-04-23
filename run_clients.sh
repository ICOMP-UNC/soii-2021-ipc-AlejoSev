for ((i = 0; i<$3; i++))
do
    ./bin/client $1 $2 $i &
    sleep .001
done