# compile
gcc answer.c -o test -mavx2 

# run for 10000 times & record
> $f.result
for _ in {1..10000}; 
do
    ./test >> $f.result
done

# remove used files
rm test

