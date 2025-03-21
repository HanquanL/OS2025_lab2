Some tips on how to run this.

The lab2 comes with two reference output directories
1) refout
2) refout_vp

# the <refout> directory is THE reference output  (292K)
# it was created by running "./runit.sh refout_vp sched"

# the <refout_vp> directory is the verbose reference output  (4268K)
# it was created with running "./runit.sh refout_vp sched -v -p"
# you don't have to create this output but it could be helpful
# in debugging

# write your program .. standard way of writing a program
mkdir src
vi sched.cpp
make 

# run all examples ... assuming your executable is in ../src/sched
# make a outputdir
mkdir outputdir
./runit.sh outputdir ../src/sched

# compare outputs with reference output
# make sure that the refout is the first argument
# because we are putting the LOG.txt into the outputdir

./gradeit.sh refout outputdir

# don't forget to include the make.log and gradeit.log
You might get something like this.

frankeh@linserv1 > ./runit.sh ./studx ../src/mysched     # creates your outputs

frankeh@linserv1 > ./gradeit.sh ./refout ./studx
in    F    L    S   R2   R5   P2   P5:3 E2:5 E4
00    1    1    0    1    1    1    1    1    1
01    1    1    1    1    1    1    1    1    1
02    1    1    1    1    1    1    1    1    1
03    1    1    1    1    1    1    1    1    1
04    1    1    1    1    1    1    1    1    1
05    1    1    1    1    1    1    1    1    1
06    1    1    1    1    1    1    1    1    1
07    1    1    1    1    0    1    0    0    1

SUM   8    8    7    8    7    8    7    7    8 
68 out of 72 correct

You have to inspect what goes wrong .. you see that input0 algo "S" failed
Go to studx/LOG.txt which will only show the command and SUM lines.
The SUM might be correct, but there are differences. Execute the diff command
listed in the LOG.txt file manually and you will see all differences

frankeh@linserv1> diff ./refout/out_0_S ./studx/out_0_S
2,4c2,4
< 0000:    0  100   10   10 2 |   201   201   101     0
< 0001:  500  100   20   10 4 |   627   127    27     0
< SUM: 627 31.90 20.41 164.00 0.00 0.319
---
> 0000:    0  100   10   10 2 |   209   201   101     0
> 0001:  500  100   20   10 4 |   622   127    27     0
> SUM: 627 31.90 20.41 164.00 0.00 0.301


In that case you need to inspect the outputs and determine why you get
different results.
You might want to run with -v option and compare in detail one particular
output and go from there. The -v references are provided in ./refout_v

frankeh@linserv1 > ./runit.sh ./studx ./src/mysched -v  # creates your outputs with -v option


Finally: 

Please go back to the first class and the projects /home/frankeh/Public/ProgExamples/Format
to see how to properly format C and C++ output to avoid rounding errors etc.

