# Lab work <mark>4</mark>: <mark>Integral calculation of the given function using threadsafe queue</mark>
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
python3 -m pip install -r requirements.txt 
```

At this point you should be good to use our python script!

### Usage

#### 1. Use c++ executable
In order to use our program via c++ executable, you have to write following command from the project root:
```shell
bin/integrate_parallel_queue "function number from 1 to 3" data/"config file name for your function" "number of threads" "points per thread"
```

Example
```shell
bin/integrate_parallel_queue 1 data/func1.cfg 2 30
```

#### 2. Use python script
```shell
python3 scripts/script.py "number of launches for every function" "number of threads" "points per thread"
```

Example
```shell
python3 scripts/script.py 5 4 30
```

#### 3. Use run_script.sh to launch python script for different amount of threads

Before using it, make sure you launch it from the project root folder
```shell
./run_script.sh "number of launches for every function" "points per thread"
```

Example 
```shell
./run_script.sh 5 20
```

### Important!

After running python script, it will create with_queue.json file in results folder inside the data folder, which saves every script launch, make sure that results folder exists! 
It can be used for debugging or saving data for future analysis.

### Results

[Here is a google drive link for a pdf with results and analysis](https://docs.google.com/document/d/1BMwZjNcKVpPiB1ad1apyNIj-mXTDbkEVI367CRfTI-E/edit)
