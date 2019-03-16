TARGET=./obj64/gauss.exe

# for s in 2048 4096
# do
#     echo "Running at size=" $s
#     for i in `seq 5`
#     do
#         $TARGET -n $s
#     done
# done

for t in 2 4 6 8 10 12 14 16 18 20
do
	$TARGET -n2048 -g2 -t $t
done

for t in 2 4 6 8 10 12 14 16 18 20
do
	$TARGET -n4096 -g4 -t $t
done

# TODO: you will want to make the testing more rigorous, for example by allowing
#       to change the number of threads