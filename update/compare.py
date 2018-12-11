import os
import hashlib
import json
from collections import OrderedDict

key_arr = []

veraport_path = 'C:\Program Files\Wizvera\Veraport20'
#veraport_path = "/mnt/c/Program Files/Wizvera/Veraport20/"

def compare():
	if not os.path.isdir(veraport_path):
		return -1
	
	client_data = get_hash()
	#print(client_data)
	#print('')
	with open('combine.json') as json_file:
		json_data = json.load(json_file)
		for key in json_data.keys():
			#print(key)
			#print(json_data[key])
			if client_data == json_data[key]:
				#print(key)
				key_arr.append(key)
			#for i in range(len(json_data[key])):
	#print(key_arr)
	return key_arr
				

def get_hash():
	hash_arr = []
	
	files = next(os.walk(veraport_path))[2]
	#print(files)
	
	for i in range(len(files)):
		temp_obj = {}
		if 'unins' in files[i] :
			continue

		#elif '.dll' in files[i]:
		#	continue
		#print(files[i])
		f = open(veraport_path+'\\'+files[i],"rb")
		data = f.read()
		hash = hashlib.sha256(data).hexdigest()
		temp_obj = {files[i]:hash}
		hash_arr.append(temp_obj)

	return hash_arr

def find_website():
	result = compare()
	#print(result)
	result_arr =[]

	if len(result) :
		for i in range(len(result)):
			key = result[i]
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

	result_obj ={"bank":result_arr}
	#print(result_obj)
	return result_obj

def make_json_file():
	data = find_website()
	print(data)
	with open('compare.json','w') as f:
		json.dump(data, f, ensure_ascii = False, indent='\t')


if __name__ == '__main__':
	#print(veraport_path)
	#compare()
	make_json_file()
	#find_website()

	