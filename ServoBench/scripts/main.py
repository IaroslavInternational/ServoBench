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
    # Находим строку с заголовками
    target_headers = ['Время', 'Температура 1', 'Температура 2', 
                     'Влажность', 'Ток', 'Напряжение', 'Энкодер', 'Момент']
    
    for i, line in enumerate(lines):
        if all(header in line for header in target_headers):
            headers = [h.strip() for h in line.split('\t')]
            data_start = i + 1
            break
    else:
        raise ValueError("Не найдена строка с заголовками")
    
    # Собираем данные
    data = {h: [] for h in headers}
    for line in lines[data_start:]:
        values = line.replace(',', '.').split('\t')
        for header, val in zip(headers, values):
            try:
                data[header].append(float(val))
            except ValueError:
                data[header].append(np.nan)
    
    # Конвертируем в numpy array для удобства
    for key in data:
        data[key] = np.array(data[key])
    
    return data

def plot_graph(data, output_path, x_limits=None):
    """
    Построение и сохранение графиков в отдельных subplot
    x_limits: кортеж (x_min, x_max) для ограничения по оси X
    """
    # Создаем фигуру с несколькими subplots
    num_plots = len(data) - 1  # Минус колонка 'Время'
    fig, axes = plt.subplots(num_plots, 1, figsize=(12, 3 * num_plots),
                            sharex=True)  # Общая ось X
    
    # Если только один график, axes будет не массивом, а объектом
    if num_plots == 1:
        axes = [axes]
    
    time = data['Время']
    del data['Время']  # Удаляем время из данных для графиков
    
    # Автоматическое определение лимитов по X, если не заданы
    if x_limits is None:
        x_min, x_max = time.min(), time.max()
    else:
        x_min, x_max = x_limits
    
    # Рисуем каждый график в своем subplot
    for ax, (name, values) in zip(axes, data.items()):
        ax.plot(time, values, label=name, color='blue')
        ax.set_ylabel(name, fontsize=10)
        ax.grid(True, linestyle='--', alpha=0.7)
        ax.legend(loc='upper right')
        ax.set_xlim(x_min, x_max)  # Устанавливаем лимиты по X
    
    # Общие настройки
    axes[-1].set_xlabel('Время, с', fontsize=12)
    plt.suptitle(os.path.basename(output_path), y=1.02)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()

def main():
    if len(sys.argv) < 2:
        print("Использование: python script.py <input_file.txt> [output_file.png] [x_min] [x_max]")
        print("Пример: python script.py data.txt output.png 84.15 84.17")
        return
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    # Парсим лимиты по X, если они заданы
    x_limits = None
    if len(sys.argv) > 3:
        try:
            x_min = float(sys.argv[3])
            x_max = float(sys.argv[4]) if len(sys.argv) > 4 else x_min + 0.1
            x_limits = (x_min, x_max)
        except ValueError:
            print("Ошибка: x_min и x_max должны быть числами")
            return
    
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
        plot_graph(data, output_file, x_limits)
        print(f"График успешно сохранен: {output_file}")
        if x_limits:
            print(f"Ось X ограничена: {x_limits[0]:.2f} - {x_limits[1]:.2f}")
    
    except Exception as e:
        print(f"Ошибка: {str(e)}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()