# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def read_file(filename):
    """Чтение файла с автоматическим определением кодировки"""
    encodings = ['utf-8-sig', 'cp1251', 'utf-16', 'iso-8859-1']
    for encoding in encodings:
        try:
            with open(filename, 'r', encoding=encoding) as f:
                return [line.strip() for line in f if line.strip()]
        except UnicodeDecodeError:
            continue
    raise ValueError(f"Не удалось прочитать файл {filename}")

def parse_data(lines):
    """Разбор данных и извлечение значений"""
    # Находим строку с заголовками (ищем строку, содержащую все нужные столбцы)
    target_headers = ['Время', 'Температура 1', 'Температура 2', 'Влажность', 'Ток', 'Напряжение', 'Энкодер', 'Момент']
    
    for i, line in enumerate(lines):
        if all(header in line for header in target_headers):
            headers = [h.strip() for h in line.split('\t')]
            data_start = i + 1
            break
    else:
        raise ValueError("Не найдена строка с заголовками. Проверьте формат файла.")
    
    # Собираем данные
    data = {h: [] for h in headers}
    for line in lines[data_start:]:
        values = line.replace(',', '.').split('\t')
        for header, val in zip(headers, values):
            try:
                data[header].append(float(val))
            except ValueError:
                data[header].append(np.nan)
    return data

def plot_graph(data, output_path):
    """Построение и сохранение графика"""
    plt.figure(figsize=(14, 8))
    time = data['Время']
    
    # Создаем дополнительные оси для разных масштабов
    fig, host = plt.subplots(figsize=(14, 8))
    axes = [host] + [host.twinx() for _ in range(3)]
    
    # Цвета для графиков
    colors = ['blue', 'green', 'red', 'purple', 'orange', 'brown']
    
    # Рисуем каждый график на соответствующей оси
    for i, (name, values) in enumerate(data.items()):
        if name == 'Время':
            continue
        ax = axes[i % len(axes)]
        ax.plot(time, values, label=name, color=colors[i % len(colors)])
        ax.set_ylabel(name, color=colors[i % len(colors)])
        ax.tick_params(axis='y', labelcolor=colors[i % len(colors)])
    
    host.set_xlabel('Время, с', fontsize=12)
    host.grid(True, linestyle='--', alpha=0.7)
    
    # Объединяем легенды
    lines, labels = [], []
    for ax in axes:
        ax_lines, ax_labels = ax.get_legend_handles_labels()
        lines.extend(ax_lines)
        labels.extend(ax_labels)
    host.legend(lines, labels, loc='upper left', bbox_to_anchor=(1.1, 1))
    
    plt.title(os.path.basename(output_path), pad=20)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()

def main():
    if len(sys.argv) < 2:
        print("Использование: python script.py <input_file.txt> [output_file.png]")
        return
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    try:
        # Чтение и обработка данных
        lines = read_file(input_file)
        data = parse_data(lines)
        
        # Определение выходного пути
        if not output_file:
            os.makedirs('Графики', exist_ok=True)
            base_name = os.path.splitext(os.path.basename(input_file))[0]
            output_file = f'Графики/{base_name}.png'
        
        # Построение графика
        plot_graph(data, output_file)
        print(f"График успешно сохранен: {output_file}")
    
    except Exception as e:
        print(f"Ошибка: {str(e)}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()