
# -*- coding: utf-8 -*-
# [+] Kali-KM		kali-km.tistory.com
# [+] Usage : prefetch_parser.py <input dir>

import decom_xpress

import unicodedata
import sys, os, datetime
from datetime import datetime, timedelta


def main():
    path = "C:\\Windows\\Prefetch"
    for path, dirs, files in os.walk(path):
        for file in files:
            if os.path.splitext(file)[1].lower() in ['.pf']:
                target = path + '//' + file
                data = decom(target)
                parser(data, target)



def decom(input_file):
     data = decom_xpress.decom(input_file)
     return data



def LittleEndianToInt(buf):
    val = 0
    for i in range(0, len(buf)):
        multi = 1
        for j in range(0, i):
            multi *= 256
        val += buf[i] * multi
    return val

def time_convert(time):
    dd = time
    dt = '%016x' % dd  #int정수형을 8바이트 정수형에 맞게 변환
    us = int(dt, 16) / 10.
    return datetime(1601, 1, 1) + timedelta(microseconds=us) + timedelta(hours=9)


def time_change(buf):
    littleendian = buf
    bigendian = LittleEndianToInt(littleendian)
    return time_convert(int(bigendian))

def remove_null(bytearray_str):
    length = len(bytearray_str)
    for i in range(0, int(length/ 2)):
        bytearray_str.remove(0)
    return bytearray_str


def remove_uni_null(uni_str):
    tmp = []
    for i in range(0, len(uni_str)):
        if uni_str[i] != u'\x00':
            tmp.append(uni_str[i])
    return ''.join(tmp)


def unicode_split(uni_str):
    uni_str = uni_str.split(u'\x00\x00')
    return uni_str


def loaded_file(buf):
    file_list_offset = LittleEndianToInt(buf[100:104])
    file_list_size = LittleEndianToInt(buf[104:108])
    file_path_block = buf[file_list_offset:file_list_offset + file_list_size]

    file_path_block = str(file_path_block)
    filepaths = unicode_split(file_path_block)
    return filepaths


def parser(buf, target):

    f = open("prefetch.csv", 'a', encoding="utf-8")

    file_name = remove_null(buf[16:49])  # File name
    file_size = LittleEndianToInt(buf[12:15])  # File Size

    last_runtime = time_change(buf[128:136])  # last_runtime = buf[128:136]
    run_conut = LittleEndianToInt(buf[208:212])

#    ctime = datetime.fromtimestamp(os.path.getctime(target))
#    mtime = datetime.fromtimestamp(os.path.getmtime(target))
    filepaths = loaded_file(buf)

    print('%s,%s,%s' % (file_name.decode(), last_runtime.date(), run_conut))
    f.write('%s,%s,%s\n' % (file_name.decode(), last_runtime.date(), run_conut))

#   date() 가 없으면 시간까지 출력

if __name__ == '__main__':
    main()