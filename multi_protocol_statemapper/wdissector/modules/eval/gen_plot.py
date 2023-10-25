#!/usr/bin/env python3

import sys
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import pandas as pd
import numpy as np
import os

# %%

X_LABEL = 'Iteration'
Y_LABEL = 'Crashes + Deadlocks'
linewidth = 1.6


def filter_data(data, max_iterations):
    # Truncate data1 to the minimum number of iterations
    i = 0
    for d in data[X_LABEL]:
        if d > max_iterations:
            data = data.truncate(after=i-1)
            break
        i += 1

    # Filter data to focus only show entries with changed crashes + deadlocks
    c_data = []
    c_data.append([data[X_LABEL][0], data[Y_LABEL][0]])
    i = 0
    last_crash_count = 0

    # Force pyplot to exclude 0 at origin and start at iteration 1
    data[X_LABEL] = data[X_LABEL] + 1

    for value in data[Y_LABEL]:

        # Last iteration
        if i == len(data[Y_LABEL]):
            c_data.append([data[X_LABEL][i], data[Y_LABEL][i]])
            break

        if value != last_crash_count:
            last_crash_count = value
            if i >= 1 and (data[X_LABEL][i-1] != data[X_LABEL][i]-1):
                c_data.append([data[X_LABEL][i]-1, data[Y_LABEL][i-1]])
            c_data.append([data[X_LABEL][i], data[Y_LABEL][i]])

        i += 1

    data = pd.DataFrame(c_data,
                        columns=['Iteration', 'Crashes + Deadlocks'])

    # Normalize data to max_iterations
    if (data[X_LABEL][len(data[X_LABEL])-1] != max_iterations):
        data = data.append({'Iteration': max_iterations,
                            'Crashes + Deadlocks': data[Y_LABEL][len(data[Y_LABEL])-1]},
                           ignore_index=True)

    return data


if __name__ == '__main__':

    script_path = sys.path[0]
    os.chdir(script_path)

    data1 = pd.read_csv('all.csv', sep=",")
    data2 = pd.read_csv('dup.csv', sep=",")
    data3 = pd.read_csv('mut.csv', sep=",")
    data4 = pd.read_csv('evo.csv', sep=",")

    max_iterations_count = [data1[X_LABEL][len(data1[X_LABEL])-1], data2[X_LABEL][len(data2[X_LABEL])-1],
                            data3[X_LABEL][len(data3[X_LABEL])-1], data4[X_LABEL][len(data4[X_LABEL])-1]]

    print(max_iterations_count)
    sys.exit(0)
    max_iterations = min(max_iterations_count)

    data1 = filter_data(data1, max_iterations)
    data2 = filter_data(data2, max_iterations)
    data3 = filter_data(data3, max_iterations)
    data4 = filter_data(data4, max_iterations)

    max_crashes = max(data1[Y_LABEL][len(data1[Y_LABEL])-1],
                      data2[Y_LABEL][len(data2[Y_LABEL])-1])

    print('plot all.csv:')
    print(data1)
    print('plot dup.csv:')
    print(data2)
    print('plot mut.csv:')
    print(data3)
    print('plot evo.csv:')
    print(data4)

    print('Samples: %d' % (data1[X_LABEL].count()))

    plt.plot(data2[X_LABEL], data2[Y_LABEL],
             linewidth=linewidth, linestyle='--', label='Duplication',
             drawstyle='steps')

    plt.plot(data3[X_LABEL], data3[Y_LABEL],
             linewidth=2.2, linestyle='dotted', label='Mutation',
             drawstyle='steps', color='black')

    plt.plot(data4[X_LABEL], data4[Y_LABEL],
             linewidth=linewidth, linestyle='-.', label='Evolution',
             drawstyle='steps')

    # plt.grid(alpha=0.2)
    plt.plot(data1[X_LABEL], data1[Y_LABEL],
             linewidth=linewidth, linestyle='-', label='All',
             drawstyle='steps', marker='x')

    # plt.xscale('log')
    plt.legend(labelspacing=0.7, loc='upper left', ncol=4, bbox_to_anchor=(-0.02, 1.20),
               fancybox=True, frameon=False,
               # prop={'weight':'bold'})
               )
    plt.ylabel('#Crash + #Deadlock', fontweight='bold')
    # plt.xlabel('Fuzzing Iterations', fontweight='bold')
    plt.xlabel('Fuzzing Iterations')

    ax = plt.gca()
    # ax.set_yticks([2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24])
    plt.xticks(np.arange(0, max_iterations, 50))
    plt.yticks(np.arange(0, max_crashes+1, 1))
    plt.xlim([1, max_iterations])
    # plt.ylim([-1, 26])
    ax.grid(alpha=0.2)
    plt.subplots_adjust(top=0.8)

    matplotlib.rc('pdf', fonttype=42)
    plt.savefig('graph_optimization.pdf', transparent=True,
                bbox_inches='tight', pad_inches=0)
    print('Figure saved as graph_optimization.pdf')
    # Show figure maximized
    figManager = plt.get_current_fig_manager()
    figManager.window.showMaximized()
    plt.show()
