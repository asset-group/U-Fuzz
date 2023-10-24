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
