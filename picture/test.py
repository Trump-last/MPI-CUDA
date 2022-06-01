'''
Autor: XLF
Date: 2022-05-30 13:55:48
LastEditors: XLF
LastEditTime: 2022-06-01 21:00:30
Description: 
'''
import matplotlib.pyplot as plt
import pandas as pd

xian_path = './8xian.csv'
cuda_path = './cuda.csv'
dan_path = './dan.csv'

def read(path):
    '''
    读取数据，并且组成数对
    '''
    data = pd.read_csv(path)
    X = data['size'].values
    Y = data['time'].values
    return X.tolist(), Y.tolist()

if __name__ == '__main__':
    dan_X,dan_Y = read(dan_path)
    xian_X,xian_Y = read(xian_path)
    cuda_X,cuda_Y = read(cuda_path)
    plt.figure()
    plt.plot(dan_X,dan_Y,color='red',marker='o',label='SEQ')
    plt.plot(xian_X,xian_Y,color='blue',marker='o', label='MPI')
    plt.plot(cuda_X,cuda_Y,color='green',marker='o', label='CUDA')    
    plt.text(7000000,2200, 'SEQ')
    plt.text(7000000,1100, 'MPI')
    plt.text(6500000,40, 'CUDA')
    plt.xlabel('number')
    plt.ylabel('time(ms)')
    for a, b in zip(dan_X[-3:], dan_Y[-3:]):
        plt.text(a, b, b, ha='center', va='bottom', fontsize=10)
    for a, b in zip(xian_X[-3:], xian_Y[-3:]):
        plt.text(a, b, b, ha='center', va='bottom', fontsize=10)
    for a, b in zip(cuda_X[-3:], cuda_Y[-3:]):
        plt.text(a, b, b, ha='center', va='bottom', fontsize=10)
    plt.show()
