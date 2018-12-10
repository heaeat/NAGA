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
	print(result)
	result_arr =[]

	if len(result) :
		for i in range(len(result)):
			key = result[i]
			if key == "busanbank":
				result_arr.append({"부산은행" :"https://ibank.busanbank.co.kr"})
			elif key == "citibank":
				result_arr.append({"시티은행":"https://www.citibank.co.kr"})
			elif key == "dgb":
				result_arr.append({"DGB대구은행":"https://www.dgb.co.kr"})
			elif key == "ibk":
				result_arr.append({"IBk기업은행":"https://www.ibk.co.kr"})
			elif key == "jbbank":
				result_arr.append({"전북은행":"https://www.jbbank.co.kr"})
			elif key == "kbstar":
				result_arr.append({"KB국민은행":"https://www.kbstar.com"})
			elif key == "kdb":
				result_arr.append({"KDB산업은행":"https://www.kdb.co.kr"})
			elif key == "kebhana":
				result_arr.append({"keb하나은행":"https://www.kebhana.com"})
			elif key == "kjbank":
				result_arr.append({"광주은행":"https://pib.kjbank.com"})
			elif key == "nonghyup":
				result_arr.append({"NHBank(농협)","https://banking.nonghyup.com"})
			elif key == "shinhan":
				result_arr.append({"신한은행","https://www.shinhan.com"})
			elif key == "standardchartered":
				result_arr.append({"SC제일은행":"https://www.standardchartered.co.kr"})
			elif key == "wooribank":
				result_arr.append({"우리은행","https://www.wooribank.com"})


	#print(result_arr)
	return result_arr

def make_json_file():
	data = find_website()
	with open('compare.json','w') as f:
		json.dump(data, f, ensure_ascii = False, indent='\t')


if __name__ == '__main__':
	#print(veraport_path)
	#compare()
	make_json_file()

	