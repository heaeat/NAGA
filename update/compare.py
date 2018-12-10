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
				print(key)
				key_arr.append(key)
			#for i in range(len(json_data[key])):
	print(key_arr)
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

def make_json_file():
	data = compare()
	with open('compare.json','w') as f:
		json.dump(data, f, ensure_ascii = False, indent='\t')

if __name__ == '__main__':
	#print(veraport_path)
	#compare()
	make_json_file()