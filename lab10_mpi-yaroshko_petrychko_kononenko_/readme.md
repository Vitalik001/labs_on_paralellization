# Lab work <mark>10</mark>: <mark>MPI</mark>
Authors (team): <mark>[Кононенко Назар](https://github.com/nazar12314), [Ярошко Тарас](https://github.com/tyaroshko), [Петричко Віталій](https://github.com/Vitalik001)</mark><br>

## Prerequisites

```
gcc, CMake, boost
```

### Compilation

<mark>Use
```shell
./compile.sh
```
in the root directory to compile the C++ executable</mark>

### Installation

First, create Python virtual environment and activate it:
```shell
python3 -m venv venv
source venv/bin/activate
```

Install all necessary dependencies with:
```shell
python3 -m pip install -r requirements.txt 
```

And now you are good to go when it comes to using our Python script!

### Usage

Create a data directory to store all the images and animation
```shell
mkdir data
```

1. Use C++ executable
   To run the program via C++ executable, write following command in the root directory of the project:
```shell
./bin/template 
```

#### Example
```shell
bin/template
```

2. Using Python script
```shell
python3 scripts/script.py "number_of_workers"
```

#### Example
```shell
python3 scripts/script.py 2
```


### Results

[Google Drive folder with testing data, results and analysis](https://docs.google.com/document/d/1RNRm6Cff7IN5wfcRfpYtRVPcxO4k2BflDcchXsEGh3Q/edit?usp=sharing)
