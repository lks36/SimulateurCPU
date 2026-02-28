compilation=$(make clean  2>&1;make test  2>&1)
if [ -f "test.exe" ] ; then
    ./test.exe 
else 
    if [ -f "test" ]; then
        ./test
    else
        echo "Erreur compilation\n $compilation"
    fi
fi
make clean > /dev/null