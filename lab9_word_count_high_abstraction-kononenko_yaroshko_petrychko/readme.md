# Lab <mark>9</mark>: <mark>Parallel word counting using tbb::parallel_pipeline</mark>
Authors (team): <mark>[Кононенко Назар](https://github.com/nazar12314), [Ярошко Тарас](https://github.com/tyaroshko), [Петричко Віталій](https://github.com/Vitalik001)</mark><br>

## Prerequisites

```
gcc, CMake, boost, libarchive, tbb
```

Preferable way to install TBB:
```shell
# Do our experiments in /tmp
cd /tmp
# Clone oneTBB repository
git clone https://github.com/oneapi-src/oneTBB.git
cd oneTBB
# Create binary directory for out-of-source build
mkdir build && cd build
# Configure: customize CMAKE_INSTALL_PREFIX and disable TBB_TEST to avoid tests build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/my_installed_onetbb -DTBB_TEST=OFF ..
# Build
cmake --build .
# Install
cmake --install .
# Well done! Your installed oneTBB is in /tmp/my_installed_onetbb
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

1. Use C++ executable

   To run the program via C++ executable, write following command in the root directory of the project:
```shell
./bin/countwords_par_tbb "config_file_path"
```

#### Example
```shell
bin/countwords_par_tbb configs/config1.cfg
```

2. Using Python script
```shell
python3 scripts/script.py "number_of_launches" "clear_buffer_bool -> 1 or 0"
```

#### Example
```shell
python3 scripts/script.py 2 0
```


### Results

[Google Drive folder with testing data, results and analysis](https://docs.google.com/document/d/1V7MamRLQ34sT_zUTsHXgcVgp9DZ8NfYpBuY12UcGFD0/edit?usp=sharing)
