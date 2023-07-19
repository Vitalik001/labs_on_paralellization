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
from typing import Tuple
import json


AMOUNT_OF_FUNCTIONS = 3
MAX_ERROR = 10 ** (-7)
OS = platform.system()
warnings.filterwarnings("ignore")
EXE_FILE = "integrate_parallel_queue"


def parse_args(name: str, description: str) -> Tuple[int, int, int]:
    parser = argparse.ArgumentParser(prog=name, description=description)
    parser.add_argument("n", type=int)
    parser.add_argument("threads", type=int)
    parser.add_argument("points_per_thread", type=int)

    args = parser.parse_args()
    return args.n, args.threads, args.points_per_thread


def run_cpp(method: int, config_file: str, threads: int, points_per_thread: int) -> list[float]:
    exe_path = os.path.join("bin", EXE_FILE)
    process = Popen([exe_path, str(method), config_file, str(threads), str(points_per_thread)], stdout=PIPE)

    output, error = process.communicate()

    values = [float(value) for value in output.decode("UTF-8").split("\n") if value]

    process.kill()
    process.terminate()

    return values


def get_results(n: int, threads: int, points_per_thread: int):
    config_file_template = "data/func%d.cfg"
    result = {}

    for i in range(1, AMOUNT_OF_FUNCTIONS+1):
        config_file = config_file_template.replace("%d", str(i))
        func_stats = defaultdict(list)

        for _ in range(n):
            res, a_err, r_err, time = run_cpp(i, config_file, threads, points_per_thread)
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
            result[i] = {
                "res_mean": mean(func_stats["result"]),
                "a_error": mean(func_stats["a_error"]),
                "r_error": mean(func_stats["r_error"]),
                "min_time": min(func_stats["time"]),
                "mean_time": mean(func_stats["time"]),
                "sd": stdev(func_stats["time"])
            }
        else:
            result[i] = {
                "res_mean": mean(func_stats["result"]),
                "err_message": "Obtained results do not differ under the giver error!"
            }

    return result


def print_results(results):
    for key in results:
        for item in results[key]:
            print(results[key][item])
        print()


def record_results(results, threads: int, points_per_thread: int):
    if not os.path.exists(f"data/results/with_queue_{points_per_thread}.json"):
        file = open(f"data/results/with_queue_{points_per_thread}.json", "x")
        file.close()
    try:
        with open(f"data/results/with_queue_{points_per_thread}.json") as file:
            data = json.load(file)
    except json.JSONDecodeError:
        data = {}

    data.update({str(datetime.datetime.now()): {"threads": threads, "results": results}})

    with open(f"data/results/with_queue_{points_per_thread}.json", "w") as file:
        json.dump(data, file)


def main():
    n, threads, points_per_thread = parse_args("Parser", "CLA parser for lab2")

    if n < 1:
        print("Number of iterations should not be less than 1")
        exit(64)

    result = get_results(n, threads, points_per_thread)
    record_results(result, threads, points_per_thread)
    print_results(result)


if __name__ == "__main__":
    main()
