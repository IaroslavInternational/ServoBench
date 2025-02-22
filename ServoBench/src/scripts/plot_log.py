import matplotlib.pyplot as plt
import numpy

HEADER_LINE = 4
DATA_LINE   = 6

filename = 'log_500.txt'

def log2dict(filename: str):
    data = None
    with open(filename, 'r') as file:
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

# Proc
out = log2dict(filename)

time_series = out['Время']
out.pop('Время')

# Plot

plt.clf()

fig, axs = plt.subplots(len(out), 1, layout='constrained')
axs[0].set_title(filename.replace('.txt',''))

for (header, arr), ax in zip(out.items(), axs):
    z = numpy.polyfit(time_series, arr, 2)
    p = numpy.poly1d(z)
    ax.plot(time_series, arr, label=header)
    ax.plot(time_series, p(arr), '--', alpha=0.7)
    ax.set_xlim(time_series[0], time_series[-1])
    ax.grid()    
    ax.legend()
    ax.set_xlabel(f'Время, с')


plt.show()