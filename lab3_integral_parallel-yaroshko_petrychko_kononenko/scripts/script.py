import argparse
import os
from subprocess import Popen, PIPE
from statistics import stdev, mean
from itertools import combinations, chain
import platform
import numpy as np
import warnings
from collections import defaultdict
import datetime
import pandas as pd
import matplotlib.pyplot as plt


AMOUNT_OF_FUNCTIONS = 3
MAX_ERROR = 10 ** (-7)
OS = platform.system()
warnings.filterwarnings("ignore")
EXE_FILE = "integrate_parallel"


def parse_args(name: str, description: str) -> int:
    parser = argparse.ArgumentParser(prog=name, description=description)
    parser.add_argument("n", type=int)
    parser.add_argument("threads", type=int)

    args = parser.parse_args()
    return args.n, args.threads


def run_cpp(method: int, config_file: str, threads: int) -> list[float]:
    exe_path = os.path.join("bin", EXE_FILE)
    process = Popen([exe_path, str(method), config_file, str(threads)], stdout=PIPE)

    output, error = process.communicate()

    values = [float(value) for value in output.decode("UTF-8").split("\n") if value]

    process.kill()
    process.terminate()

    return values


def get_results(n: int, threads: int):
    config_file_template = "data/func%d.cfg"
    result = {}

    for i in range(1, AMOUNT_OF_FUNCTIONS+1):
        config_file = config_file_template.replace("%d", str(i))
        func_stats = defaultdict(list)

        for _ in range(n):
            res, a_err, r_err, time = run_cpp(i, config_file, threads)
            func_stats["result"].append(res)
            func_stats["a_error"].append(a_err)
            func_stats["r_error"].append(r_err)
            func_stats["time"].append(time)

        if n == 1:
            result[i] = list(chain(*func_stats.values()))
            result[i] += [result[i][-1], 0]
            continue

        res_pairs = np.array(list(combinations(func_stats["result"], 2)))

        if all(np.abs(res_pairs[:, 0] - res_pairs[:, 1]) < MAX_ERROR):
            result[i] = [
                mean(func_stats["result"]),
                mean(func_stats["a_error"]),
                mean(func_stats["r_error"]),
                min(func_stats["time"]),
                mean(func_stats["time"]),
                stdev(func_stats["time"])
            ]
        else:
            result[i] = [
                mean(func_stats["result"]),
                "Obtained results do not differ under the giver error!"
            ]

    return result


def print_results(results):
    for key in results:
        for item in results[key]:
            print(item)
        print()


def record_results(results, n: int, threads: int):
    with open("data/history.txt", "a") as file:
        file.write(f"\n------ Recording for : {datetime.datetime.now()} ------ \n")
        file.write(f"For n = {n} \n")
        file.write(f"Using {threads} threads \n")

    results = np.array(list(results.values()))

    df = pd.DataFrame(results, columns=["sum", "abs_err", "rel_err", "min_time", "average_time", "sd"])

    df.to_csv("data/history.txt", mode="a")


def main():
    n, threads = parse_args("Parser", "CLA parser for lab2")

    if n < 1:
        print("Number of iterations should not be less than 1")
        exit(64)

    result = get_results(n, threads)
    record_results(result, n, threads)
    print_results(result)


if __name__ == "__main__":
    main()
