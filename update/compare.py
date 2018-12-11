#test.py
import os
import sys
import hashlib
import json
from collections import OrderedDict


result_key_arr = []

def get_client_data(path):
	if not os.path.isdir(path):
		print("폴더가 음썰...")
		return -1

	hash_arr = []
	file_list = next(os.walk(path))[2]
	#print(files)
	
	return get_hash(path, file_list)

def get_hash(path, file_list):
	hash_arr = []
	#print(file_list)
	for i in range(len(file_list)) :
		if 'unins' in file_list[i]:
			continue
		
		temp_obj = {}

		#print(file_list[i])
		f = open(path+file_list[i],"rb")
		data = f.read()
		hash = hashlib.sha256(data).hexdigest()
		#print(hash)
		#print('')
		temp_obj = {file_list[i]:hash}
		#print(temp_obj)
		hash_arr.append(temp_obj)

	#print('')
	#print(hash_arr)

	return hash_arr

def compare():
	
	client_data = get_client_data(path)

	with open('combine.json') as json_file:
		combine_json_data = json.load(json_file)
		for key in combine_json_data.keys():
			if client_data == combine_json_data[key]:
				result_key_arr.append(key)

	return result_key_arr

def find_website():

	result_key = compare()
	#print(result_key)

	result_arr = []

	if len(result_key):
		for i in range(len(result_key)):
			
			key = result_key[i]

			if key == "busanbank":
				result_arr.append({"name":"부산은행", "site":"https://ibank.busanbank.co.kr"})
			elif key == "citibank":
				result_arr.append({"name":"시티은행","site":"https://www.citibank.co.kr"})
			elif key == "dgb":
				result_arr.append({"name":"DGB대구은행","site":"https://www.dgb.co.kr"})
			elif key == "ibk":
				result_arr.append({"name":"IBk기업은행","site":"https://www.ibk.co.kr"})
			elif key == "jbbank":
				result_arr.append({"name":"전북은행","site":"https://www.jbbank.co.kr"})
			elif key == "kbstar":
				result_arr.append({"name":"KB국민은행","site":"https://www.kbstar.com"})
			elif key == "kdb":
				result_arr.append({"name":"KDB산업은행","site":"https://www.kdb.co.kr"})
			elif key == "kebhana":
				result_arr.append({"name":"keb하나은행","site":"https://www.kebhana.com"})
			elif key == "kjbank":
				result_arr.append({"name":"광주은행","site":"https://pib.kjbank.com"})
			elif key == "nonghyup":
				result_arr.append({"name":"NHBank(농협)","site":"https://banking.nonghyup.com"})
			elif key == "shinhan":
				result_arr.append({"name":"신한은행","site":"https://www.shinhan.com"})
			elif key == "standardchartered":
				result_arr.append({"name":"SC제일은행","site":"https://www.standardchartered.co.kr"})
			elif key == "wooribank":
				result_arr.append({"name":"우리은행","site":"https://www.wooribank.com"})

		result = {"bank":result_arr}

		return result

def make_json_file():
	data = find_website()
	with open('compare.json','w') as f:
		json.dump(data, f, ensure_ascii  = False, indent = '\t')

def check_path(path):
	
	if 'x86' in path:
		
		#print("은행 사이트 경로는 x86이 안들어갑니당...!!")

		return -1
	else:
		return 0

if __name__ == '__main__':
	
	path = sys.argv[1]

	if not check_path(path):
		#print(type(path))
		#print(path)
		make_json_file()


