for f in ./*.c; 
do
    echo "Processing $f..."

    # compile
    gcc $f -o test -mavx2 

    # run for 10000 times & record
    > $f.result
    for _ in {1..10000}; 
    do
        ./test >> $f.result
    done

    # calc average performance
    ./calc.py $f.result

    # remove used files
    rm $f.result
    rm test
done

