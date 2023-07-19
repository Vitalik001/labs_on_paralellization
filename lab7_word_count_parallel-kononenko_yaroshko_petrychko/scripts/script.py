import argparse
import os
from subprocess import Popen, PIPE, run
from statistics import stdev, mean
from itertools import combinations, chain
import platform
import numpy as np
import warnings
from collections import defaultdict
import datetime
from typing import Tuple
import json
import pandas as pd

OS = platform.system()
warnings.filterwarnings("ignore")
EXE_FILE = "countwords_par"
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
            elif key.strip() == 'merging_threads':
                merging = value
    return indexing, merging


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
        total, finding, reading, writing = [float(i.split("=")[1]) for i in output.decode("UTF-8").split("\n")[:4]]
    except IndexError:
        print("Error in cpp file")
        exit(1)
    process.kill()
    process.terminate()
    return [total, finding, reading, writing]


def get_results(n: int, cc: bool):
    config_file = CONFIG_FILE
    result = {}
    all_info = {}
    totals = []
    findings = []
    readings = []
    writings = []

    for i in range(1, n + 1):
        times = run_cpp(config_file)
        totals.append(times[0])
        findings.append(times[1])
        readings.append(times[2])
        writings.append(times[3])

        if i == 1:
            first_n = pd.read_csv(RESULT_A_FILE, sep=r"\s+", header=None)
            first_a = pd.read_csv(RESULT_N_FILE, sep=r"\s+", header=None)
        else:
            second_n = pd.read_csv(RESULT_A_FILE, sep=r"\s+", header=None)
            second_a = pd.read_csv(RESULT_N_FILE, sep=r"\s+", header=None)

            if not (first_n.equals(second_n) and first_a.equals(second_a)):
                print(f"results of 1 and {i} run doesn't coincide")
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
    result["mean_findings"] = mean(findings)
    result["min_findings"] = min(findings)
    result["sd_findings"] = 0 if n == 1 else stdev(findings)
    result["mean_readings"] = mean(readings)
    result["min_readings"] = min(readings)
    result["sd_readings"] = 0 if n == 1 else stdev(readings)
    result["mean_writings"] = mean(writings)
    result["min_writings"] = min(writings)
    result["sd_writings"] = 0 if n == 1 else stdev(writings)
    all_info["results"] = result
    all_info["n"] = n
    return all_info


def print_results(results):
    print(results["mean_total"])
    print(results["min_total"])
    print(results["sd_total"])
    print()
    print(results["mean_findings"])
    print(results["min_findings"])
    print(results["sd_findings"])
    print()
    print(results["mean_readings"])
    print(results["min_readings"])
    print(results["sd_readings"])
    print()
    print(results["mean_writings"])
    print(results["min_writings"])
    print(results["sd_writings"])


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
                "indexing": results["indexing"],
                "merging":results["merging"]

            }
        }
    )

    with open(f"data/results/countwords.json", "w") as file:
        json.dump(data, file)


def main():
    n, cc = parse_args("Parser", "CLA parser for lab2")
    indexing, merging = get_threads_number(CONFIG_FILE)
    if n < 1:
        print("Number of iterations should not be less than 1")
        exit(64)

    result = get_results(n, cc)
    result["indexing"] = int(indexing)
    result["merging"] = int(merging)
    # record_results(result)
    print_results(result["results"])



main()
