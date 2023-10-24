#!/usr/bin/env python3

import sys
from click import style
from cov import get_cov
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import matplotlib.patheffects as pe
from eval import *
from gen_plot import filter_data, X_LABEL, Y_LABEL


linewidth = 1.6

if __name__ == '__main__':

    script_path = sys.path[0]
    os.chdir(script_path)

    max_iterations = []
    crashes = []
    anomalies = []
    states_count = []
    lmp_states = []
    non_lmp_states = []
    coverage = []

    for i in ['1', '15', '30', '45', '60']:
        # Generate reference model
        gen_model('refs', 'refs/'+i+'.pcapng',
                  output_name=i+'.txt', model_name='refs/'+i, show_output=False)
        # Evaluate logs for each reference model evaluation
        m, c, a, s, l, n, cov = eval('eval_'+i, calculate_fitness=True, print_results=False, print_model_stats=True,
                                     ref_model_path='refs/'+i+'.json', mapper_output='refs/'+i+'.txt', show_output=False)

        max_iterations.append(m)
        crashes.append(c)
        anomalies.append(a)
        states_count.append(s)
        lmp_states.append(l)
        non_lmp_states.append(n)
        coverage.append(cov)

    print('--------------------- Graph Summary ------------------------')

    for i, m in enumerate(['1', '15', '30', '45', '60']):
        print('====== eval_' + m + ' ======')
        print('Max iterations: ' + str(max_iterations[i]))
        print('Crashes: ' + str(crashes[i]))
        print('Anomalies: ' + str(anomalies[i]))
        print('Coverage: ' + str(coverage[i]))
        print('States count: ' + str(states_count[i]))
        print('LMP states: ' + str(lmp_states[i]))
        print('Non-LMP states: ' + str(non_lmp_states[i]))

    eval_1 = pd.read_csv("eval_1.csv")
    eval_15 = pd.read_csv("eval_15.csv")
    eval_30 = pd.read_csv("eval_30.csv")
    eval_45 = pd.read_csv("eval_45.csv")
    eval_60 = pd.read_csv("eval_60.csv")

    max_iterations_count = [eval_1[X_LABEL][len(eval_1[X_LABEL])-1],
                            eval_15[X_LABEL][len(eval_15[X_LABEL])-1],
                            eval_30[X_LABEL][len(eval_30[X_LABEL])-1],
                            eval_45[X_LABEL][len(eval_45[X_LABEL])-1],
                            eval_60[X_LABEL][len(eval_60[X_LABEL])-1]]

    max_iterations = min(max_iterations_count)

    eval_1 = filter_data(eval_1, max_iterations)
    eval_15 = filter_data(eval_15, max_iterations)
    eval_30 = filter_data(eval_30, max_iterations)
    eval_45 = filter_data(eval_45, max_iterations)
    eval_60 = filter_data(eval_60, max_iterations)

    fig, (axs1, axs2) = plt.subplots(2)

    axs1.plot(
        eval_1["Iteration"],
        eval_1["Crashes + Deadlocks"],
        linewidth=linewidth,
        linestyle="--",
        label="1min.",
        drawstyle="steps",
    )

    # plt.plot(eval_5['Iteration'],eval_5['Crashes'],
    # 		linewidth=2.2, linestyle='dotted', label='5',
    # 		drawstyle='steps', color='black')

    axs1.plot(
        eval_15["Iteration"],
        eval_15["Crashes + Deadlocks"],
        linewidth=linewidth,
        linestyle="-.",
        label="15min.",
        drawstyle="steps",
    )

    axs1.plot(
        eval_30["Iteration"],
        eval_30["Crashes + Deadlocks"],
        linewidth=linewidth,
        linestyle="-",
        label="30min.",
        # drawstyle='steps', marker='.')
        drawstyle="steps",
    )

    line, = axs1.plot(
        eval_45["Iteration"],
        eval_45["Crashes + Deadlocks"],
        linewidth=linewidth,
        linestyle="-",
        label="45min.",
        # drawstyle='steps', marker='x', markevery=3
        drawstyle="steps",
    )
    line.set_dashes([5, 2, 1, 2, 1, 2])

    axs1.plot(
        eval_60["Iteration"],
        eval_60["Crashes + Deadlocks"],
        linewidth=linewidth,
        linestyle="dotted",
        label="60min.",
        # drawstyle='steps', marker='.')
        drawstyle="steps",
    )

    axs1.grid(alpha=0.3)
    # plt.margins(x=0, y=0)
    # plt.xscale('log')
    axs1.legend(
        labelspacing=0.3,
        loc="upper left",
        ncol=4,
        fancybox=True,
        frameon=False,
        # prop={"weight": "bold"},
    )
    axs1.set_ylabel("#Crash + #Deadlock", fontweight="bold")
    # axs1.set_xlabel("a) Fuzzing Iterations vs Crashes per model", fontweight="bold")
    axs1.set_xlabel("a) Fuzzing Iterations vs Crashes per model")
    # axs1.set_title('a) Fuzzing Iterations vs Crashes per model', fontweight='bold')
    axs1.set_yticks([2, 4, 8, 6, 10, 12])
    axs1.set_xlim(left=0, right=1000)
    axs1.set_ylim(top=14)

    # Stacker Bar Chart

    labels = ["1 min.", "15 min.", "30 min.", "45 min.", "60 min."]
    # Get number of crashes at last iteration
    crashes_number = crashes

    anomalies_number = anomalies

    width = 0.25
    threshold = non_lmp_states[0]

    # lmp_means = [50+13-1, 56+13-1, 61+13-1, 69+13-1, 75+13-1]
    lmp_means = [lmp_states[0], lmp_states[1],
                 lmp_states[2], lmp_states[3], lmp_states[4]]
    lmp_std = [3, 5, 2, 3, 3]

    # non_lmp_means = [34-14, 34-14, 34-14, 34-14, 34-14]
    non_lmp_means = [non_lmp_states[0], non_lmp_states[1],
                     non_lmp_states[2], non_lmp_states[3], non_lmp_states[4]]
    non_lmp_std = [2, 3, 4, 1, 2]

    # axs2.bar(labels, non_lmp_means, width, yerr=non_lmp_std,
    axs2.bar(labels, non_lmp_means, width, label="Non-LMP States",
             color="darkorange", alpha=0.5, hatch='..', edgecolor='black')
    axs2.bar(
        labels,
        lmp_means,
        width,
        bottom=non_lmp_means,
        label="LMP States",
        color="cornflowerblue",
        edgecolor='black',
        alpha=0.5,
        hatch='xx'
    )

    axs2.bar(
        labels,
        [0, 0, 0, 0, 0],
        width,
        bottom=[0, 0, 0, 0, 0],
        label='"Crash" States',
        color="darkred",
        edgecolor='black'
    )

    axs2.set_ylabel("Number of States", fontweight="bold")
    # axs2.set_ylabel("Number of States")
    # axs2.set_xlabel("b) Number of States and Crashes in each model", fontweight="bold")
    axs2.set_xlabel("b) Number of States and Crashes in each model")
    # axs2.legend()
    axs2.legend(
        labelspacing=0.1,
        loc="upper left",
        ncol=5,
        fancybox=True,
        frameon=False,
        # prop={"weight": "bold"},
    )
    axs2.grid(alpha=0.3)
    axs2.set_xlim(left=-0.5, right=4.6)
    axs2.set_ylim(top=149)

    # Lines
    axs2.plot([-0.5, 5], [threshold, threshold], "k--")
    # axs2.text(-0.40, 22, "20", fontweight="bold")
    axs2.text(-0.40, non_lmp_means[0]+2, str(non_lmp_means[0]))
    # axs2.text(0.92, 22, "34", fontweight='bold')
    # axs2.text(1.92, 22, "34", fontweight='bold')
    # axs2.text(2.92, 22, "34", fontweight='bold')
    # axs2.text(3.92, 22, "34", fontweight='bold')

    # States
    # axs2.text(-0.09, 86, "50", fontweight='bold')
    # axs2.text(0.92, 92, "56", fontweight='bold')
    # axs2.text(1.92, 97, "61", fontweight='bold')
    # axs2.text(2.92, 105, "69", fontweight='bold')
    # axs2.text(3.92, 111, "75", fontweight='bold')

    # Get coverage
    coverage_number = coverage
    # print("Coverage:")
    # for cov in coverage_number:
    #     print(cov)
    print("-" * 30)

    # Anomalies
    axs2.text(
        -0.285,
        86,
        "A:" + str(anomalies_number[0]) + "|C:" + coverage_number[0] + "%",
        # fontweight="bold",
        fontsize="smaller",
    )
    axs2.text(
        0.71,
        92,
        "A:" + str(anomalies_number[1]) + "|C:" + coverage_number[1] + "%",
        # fontweight="bold",
        fontsize="smaller",
    )
    axs2.text(
        1.71,
        97,
        "A:" + str(anomalies_number[2]) + "|C:" + coverage_number[2] + "%",
        # fontweight="bold",
        fontsize="smaller",
    )
    axs2.text(
        2.71,
        105,
        "A:" + str(anomalies_number[3]) + "|C:" + coverage_number[3] + "%",
        # fontweight="bold",
        fontsize="smaller",
    )
    axs2.text(
        3.71,
        111,
        "A:" + str(anomalies_number[4]) + "|C:" + coverage_number[4] + "%",
        # fontweight="bold",
        fontsize="smaller",
    )

    idx = 0
    axs2.plot(
        [-width / 2 + idx, width / 2 + idx],
        [threshold + lmp_means[idx] / 2, threshold + lmp_means[idx] / 2],
        "-",
        color="darkred",
        linewidth=(1 * crashes_number[idx] * 1) - 2,
        path_effects=[pe.Stroke(linewidth=(
            1 * crashes_number[idx] * 1), foreground='k'), pe.Normal()]
    )
    axs2.text(
        0.2 + idx,
        threshold + (lmp_means[idx] / 2) - 4.4,
        crashes_number[idx],
        # fontweight="bold",
    )

    idx = 1
    axs2.plot(
        [-width / 2 + idx, width / 2 + idx],
        [threshold + lmp_means[idx] / 2, threshold + lmp_means[idx] / 2],
        "-",
        color='darkred',
        linewidth=(1 * crashes_number[idx] * 1) - 2,
        path_effects=[pe.Stroke(linewidth=(
            1 * crashes_number[idx] * 1), foreground='k'), pe.Normal()]
    )
    axs2.text(
        0.2 + idx,
        threshold + (lmp_means[idx] / 2) - 4.4,
        crashes_number[idx],
        # fontweight="bold",
    )

    idx = 2
    axs2.plot(
        [-width / 2 + idx, width / 2 + idx],
        [threshold + lmp_means[idx] / 2, threshold + lmp_means[idx] / 2],
        "-",
        color="darkred",
        linewidth=(1 * crashes_number[idx] * 1) - 2,
        path_effects=[pe.Stroke(linewidth=(
            1 * crashes_number[idx] * 1), foreground='k'), pe.Normal()]
    )
    axs2.text(
        0.2 + idx,
        threshold + (lmp_means[idx] / 2) - 4.4,
        crashes_number[idx],
        # fontweight="bold",
    )

    idx = 3
    axs2.plot(
        [-width / 2 + idx, width / 2 + idx],
        [threshold + lmp_means[idx] / 2, threshold + lmp_means[idx] / 2],
        "-",
        color="darkred",
        linewidth=(1 * crashes_number[idx] * 1) - 2,
        path_effects=[pe.Stroke(linewidth=(
            1 * crashes_number[idx] * 1), foreground='k'), pe.Normal()]
    )
    axs2.text(
        0.2 + idx,
        threshold + (lmp_means[idx] / 2) - 4.4,
        crashes_number[idx],
        # fontweight="bold",
    )

    idx = 4
    axs2.plot(
        [-width / 2 + idx, width / 2 + idx],
        [threshold + lmp_means[idx] / 2, threshold + lmp_means[idx] / 2],
        "-",
        color="darkred",
        linewidth=(1 * crashes_number[idx] * 1) - 2,
        path_effects=[pe.Stroke(linewidth=(
            1 * crashes_number[idx] * 1), foreground='k'), pe.Normal()]
    )
    axs2.text(
        0.2 + idx,
        threshold + (lmp_means[idx] / 2) - 4.4,
        crashes_number[idx],
        # fontweight="bold",
    )

    plt.subplots_adjust(right=0.80)
    fig.tight_layout(pad=0.1)

    axs1.yaxis.tick_right()
    axs2.yaxis.tick_right()

    plt.rc("pdf", fonttype=42)  # embed pdf fonts (truetype)
    plt.savefig("graph_models.pdf", transparent=True,
                bbox_inches="tight", pad_inches=0)
    print('Figure saved as graph_models.pdf')
    plt.show()
