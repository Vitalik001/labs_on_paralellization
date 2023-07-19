import argparse
import os
from subprocess import Popen, PIPE, run
from statistics import stdev, mean
import platform
import warnings
import datetime
from typing import Tuple
import json
import pandas as pd

OS = platform.system()
warnings.filterwarnings("ignore")
EXE_FILE = "countwords_par_proftools"
RESULT_A_FILE = "data/res_a.txt"
RESULT_N_FILE = "data/res_n.txt"
CONFIG_FILE = "./configs/config1.cfg"


def get_threads_number(config_file):  # Replace with the actual filename
    config_data = []
    with open(config_file, 'r') as file:
        config_data = file.readlines()

    indexing, merging = 0, 0
    settings = {}
    for i, line in enumerate(config_data):
        line = line.strip()
        if line and not line.startswith('#'):
            key, value = line.split('=')
            settings[key.strip()] = value.strip()
            if key.strip() == 'indexing_threads':
                indexing = value
    return indexing

def parse_args(name: str, description: str, ) -> Tuple[int, int]:
    parser = argparse.ArgumentParser(prog=name, description=description)
    parser.add_argument("n", type=int)
    parser.add_argument("clear_cache", type=int)

    args = parser.parse_args()
    return args.n, args.clear_cache


def run_cpp(config_file: str) -> float:
    exe_path = os.path.join("bin", EXE_FILE)
    process = Popen([exe_path, config_file], stdout=PIPE)

    output, error = process.communicate()
    try:
        total, recording = [float(i.split("=")[1]) for i in output.decode("UTF-8").split("\n")[:2]]
    except Exception as e:
        print("Error in cpp file")
        exit(1)
    process.kill()
    process.terminate()
    return [total, recording]


def get_results(n: int, cc: bool):
    config_file = CONFIG_FILE
    result = {}
    all_info = {}
    totals = []
    recordings =[]

    for i in range(1, n + 1):
        times = run_cpp(config_file)
        totals.append(times[0])
        recordings.append(times[1])

        if i == 1:
            first_n = pd.read_csv(RESULT_A_FILE, sep=r"\s+", header=None)
            first_a = pd.read_csv(RESULT_N_FILE, sep=r"\s+", header=None)
        else:
            second_n = pd.read_csv(RESULT_A_FILE, sep=r"\s+", header=None)
            second_a = pd.read_csv(RESULT_N_FILE, sep=r"\s+", header=None)

            if not (first_n.equals(second_n) and first_a.equals(second_a)):
                print(f"results of 1 and {i} run doesn't coincide")
                print(first_a.compare(second_a))
                break

        if cc:
            if OS == "Linux" or OS == "Darwin":
                run(
                    [
                        "sudo",
                        "sync",
                        "sudo",
                        "sh",
                        "-c",
                        "echo",
                        "3",
                        ">",
                        "/proc/sys/vm/drop_caches",
                    ]
                )
            else:
                run(["ipconfig", "/flushdns"])

    result["mean_total"] = mean(totals)
    result["min_total"] = min(totals)
    result["sd_total"] = 0 if n == 1 else stdev(totals)
    result["mean_recordings"] = mean(recordings)
    result["min_recordings"] = min(recordings)
    result["sd_recordings"] = 0 if n == 1 else stdev(recordings)
    all_info["results"] = result
    all_info["n"] = n
    return all_info


def print_results(results):
    print(results["mean_total"])
    print(results["min_total"])
    print(results["sd_total"])
    print()
    print(results["mean_recordings"])
    print(results["min_recordings"])
    print(results["sd_recordings"])
    print()

def record_results(results):
    if not os.path.exists(f"data/results/countwords.json"):
        file = open(f"data/results/countwords.json", "x")
        file.close()
    try:
        with open(f"data/results/countwords.json") as file:
            data = json.load(file)
    except json.JSONDecodeError:
        data = {}

    data.update(
        {
            str(datetime.datetime.now()): {
                "n": results["n"],
                "results": results["results"],
                "indexing": results["indexing"]
            }
        }
    )

    with open(f"data/results/countwords.json", "w") as file:
        json.dump(data, file)


def main():
    n, cc = parse_args("Parser", "CLA parser for lab2")

    if n < 1:
        print("Number of iterations should not be less than 1")
        exit(64)
    indexing = get_threads_number(CONFIG_FILE)
    result = get_results(n, cc)
    result["indexing"] = int(indexing)
    # record_results(result)
    print_results(result["results"])



main()
