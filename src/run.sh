#!/bin/bash
solutions1=("./secuencial" "./openmp")
solutions2=("./mpi ./mixed")
# sizes=(1500 5000 7500)
# iterations=(200000 500000 1000000)
# replications=20
# iterations=10
# np=4
sizes=(15 50 75)
iterations=(20 50 100)
replications=2
iterations=10
np=4

for iter in ${iterations[@]}; do
    echo "Iteraciones:" $iter
    for size in ${sizes[@]}; do
        echo "Lado matriz:" $size
        for solu in ${solutions1[@]}; do
            echo "Solucion:" $solu
            i=1
            while [ $i -le $replications ]; do
                $solu $size $iter
                ((i++))
            done
        done
        for solu in ${solutions2[@]}; do
            echo "Solucion:" $solu
            i=1
            while [ $i -le $replications ]; do
                mpiexec -np $np $solu $size $iter
                ((i++))
            done
        done
    done
done
