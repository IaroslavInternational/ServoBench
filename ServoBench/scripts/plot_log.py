# -*- coding: cp1251 -*-

import matplotlib.pyplot as plt
import numpy
import sys
import os

HEADER_LINE = 4
DATA_LINE   = 6

def log2dict(filename: str):
    data = None
    with open(filename, 'r', encoding="utf-8") as file:
        data = file.read()
        
    data = data.split('\n')
    data.pop()

    headers = list(filter(None, data[HEADER_LINE].split('\t')))
    out = dict.fromkeys(headers) 
    for key in out.keys():
        out[key] = []

    line_c = 0
    for line in data:
        if line_c > DATA_LINE - 1:
            filtered_line = list(filter(None, line.split('\t')))
            for val, header in zip(filtered_line, headers):
                out[header].append(float(val.replace(',', '.')))
        line_c += 1

    return out

def main(filename):
    out = log2dict(filename)
    filename = filename.replace(".txt","")
    filename = filename.split("/")
    
    time_series = out['Время']
    out.pop('Время')

    plt.clf()

    fig, axs = plt.subplots(len(out), 1)
    axs[0].set_title(filename[-1])

    for (header, arr), ax in zip(out.items(), axs):
        #z = numpy.polyfit(time_series, arr, 2)
        #p = numpy.poly1d(z)
        ax.plot(time_series, arr, label=header)
        #ax.plot(time_series, p(arr), '--', alpha=0.7)
        ax.set_xlim(time_series[0], time_series[-1])
        ax.grid()    
        ax.legend(loc='upper right')
        ax.set_xlabel(f'Время, с')

    os.makedirs('Графики', exist_ok=True)
    plt.savefig(f'Графики/{filename[-1]}.png')

if __name__ == '__main__':
    arg = sys.argv[1]
    main(arg)