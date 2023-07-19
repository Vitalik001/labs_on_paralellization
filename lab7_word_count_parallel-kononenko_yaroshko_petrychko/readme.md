# Lab work <mark>7</mark>: <mark>Words parallel counting</mark>
Authors (team): <mark>[Кононенко Назар](https://github.com/nazar12314), [Ярошко Тарас](https://github.com/tyaroshko), [Петричко Віталій](https://github.com/Vitalik001)</mark><br>

## Prerequisites

<mark>GCC, CMAKE, C++ boost library, libarchive.</mark>


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

1. Use c++ executable
   In order to use our program via c++ executable, you have to write following command from the project root:
```shell
bin/countwords_par "path to config file"
```

#### Example
```shell
bin/countwords_par configs/config1.cfg
```

2. Use python script
```shell
python3 scripts/script.py "number of launches" "clear buffer or not(1/0)"
```

#### Example
```shell
python3 scripts/script.py 2 0
```

### Important!

To record results into .json file, you need to uncomment 154 line of the script.py. Obtained results would be recorded into results folder, so make sure that this folder exists.

### Results

[Here is a google drive link for a pdf with results and analysis](https://docs.google.com/document/d/1hA_XQaIYYuKtCCT_qcUrbKBVN7sXCmavtb9zsATMjZI/edit?usp=sharing)
