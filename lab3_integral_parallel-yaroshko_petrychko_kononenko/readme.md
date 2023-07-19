# Lab work <mark>2</mark>: <mark>Integral calculation of the given function</mark>
Authors (team): <mark>[Кононенко Назар](https://github.com/nazar12314), [Ярошко Тарас](https://github.com/tyaroshko), [Петричко Віталій](https://github.com/Vitalik001)</mark><br>

## Prerequisites

<mark>GCC, CMAKE, C++ boost library.</mark>


### Compilation

<mark>Use
```shell
./compile.sh
```
in the root directory to compile the c++ executable</mark>

### Installation

Firstly you need to create python virtual environment:
```shell
python3 -m venv venv
```

After that you need to activate it:
```shell
source venv/bin/activate
```

And install all dependencies by using:
```shell
python3 -m pip install-r requirements.txt 
```

At this point you should be good to use our python script!

### Usage

1. Use c++ executable
In order to use our program via c++ executable, you have to write following command from the project root:
```shell
bin/integrate_parallel "function number from 1 to 3" data/"config file name for your function" "number of threads"
```

#### Example
```shell
bin/integrate_parallel 1 data/func1.cfg 2
```

2. Use python script
```shell
python3 scripts/script.py "number of launches for every function" "number of threads"
```

#### Example
```shell
python3 scripts/script.py 5 4
```

### Important!

After running python script, it will create "history.txt" file in data folder, which saves every script launch. 
It can be used for debugging or saving data for future analysis, and can be disabled by commenting 111 line in python script

### Results

[Here is a google drive link for a pdf with results and analysis](https://docs.google.com/document/d/1pelDmrFfy8VM3y-iagxcFHC2ODL8zg5J-Ym6piBejFY/edit?usp=sharing)
