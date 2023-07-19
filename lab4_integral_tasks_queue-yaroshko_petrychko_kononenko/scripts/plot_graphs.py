from collections import defaultdict
import numpy as np
import matplotlib.pyplot as plt
import json


colors = [
    "steelblue",
    "sandybrown",
    "darksalmon",
    "lightseagreen",
    "forestgreen",
    "plum",
    "darkcyan",
    "limegreen",
    "hotpink",
    "mediumaquamarine",
    "wheat"
]


def parse_json(file):
    with open(file) as f:
        result = json.load(f)

    funcs = defaultdict(list)
    data = defaultdict(dict)

    for key in result:
        element = result[key]
        results = element["results"]

        for func in results:
            funcs[func].append(results[func])

    for func in funcs:
        data[int(func)]["min_time"] = [i["min_time"] / 1000 for i in funcs[func]]
        data[int(func)]["mean_time"] = [i["mean_time"] / 1000 for i in funcs[func]]
        data[int(func)]["sd"] = [i["sd"] / 1000 for i in funcs[func]] 

    return data


def plot_time_per_cores(queue_time, time, queue_sd, sd, i) -> None:
    X = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16]

    X_axis = np.arange(len(X))

    plt.bar(X_axis + 0.2, queue_time, 0.4, label='Average time - with queue', color="steelblue")
    plt.bar(X_axis - 0.2, time, 0.4, label='Average time - no queue', color="sandybrown")
    plt.errorbar(X_axis + 0.2, queue_time, queue_sd, linestyle='None', color="black")
    plt.errorbar(X_axis - 0.2, time, sd, linestyle='None', color="black")
    plt.xticks(X_axis, X)
    plt.xlabel("Threads")
    plt.ylabel("Time, seconds")
    plt.title(f"Function {i}")
    plt.legend()
    plt.savefig(f'data/plots/plot_func_{i}.png', bbox_inches='tight')
    plt.cla()


def plot_line_graph(X, save_into, x_label="X", y_label="Y", title="Line plot", layers=[], labels=[]):
    for idx, (layer, label) in enumerate(zip(layers, labels)):
        plt.plot(X, layer, color=colors[idx], marker='.', linestyle='solid', mec=colors[idx], label=label)

    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)
    plt.legend()
    plt.savefig(save_into, bbox_inches='tight')
    plt.cla()


def main():
    with_queue = parse_json("data/with_queue.json")
    without_queue = parse_json("data/without_queue.json")

    X = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16])

    for i in range(1, 4):
        title = f"Function {i}"
        save_into = f'data/plots/plot_eff_{i}.png'
        with_queue_time = np.array(with_queue[i]["mean_time"])
        without_queue_time = np.array(without_queue[i]["mean_time"])
        with_queue_sd = with_queue[i]["sd"]
        without_queue_sd = without_queue[i]["sd"]

        plot_time_per_cores(
            with_queue_time,
            without_queue_time,
            with_queue_sd,
            without_queue_sd,
            i
        )

        plot_line_graph(
            X,
            save_into,
            x_label="Amount of cores",
            y_label="Parallelization efficiency factor",
            title=title,
            layers=[with_queue_time[0] / X / with_queue_time, without_queue_time[0] / X / without_queue_time],
            labels=["With queue", "Without queue"]
        )
    

main()
