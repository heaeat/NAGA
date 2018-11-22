import os
import requests
import json
from collections import OrderedDict
import hashlib
#from pprint import pprint

#print(os.getcwd())

pwd = os.getcwd()
path = pwd +'/installer/'
data = OrderedDict()
arr  = []

def download_file(pwd):

	with open('install.json') as data_file:
	    data = json.load(data_file)

	make_dir()

	#pprint(data)
	for i in range(len(data["install"])):

	    install_url = data["install"][i]["route"]
	   
	    r = requests.get(install_url, allow_redirects = True)
	    #print(data["install"][i]["bank"]+'.exe')
	    file = path + data["install"][i]["bank"] + '.exe'
	    #print(file)
	    open(file,'wb').write(r.content)
	    os.system(file)

def find_hash(dir_path,data):
	try:

		filenames = os.listdir(dir_path)
		for filename in filenames:
			full_filename = os.path.join(dir_path,filename)
			if os.path.isdir(full_filename):
				find_hash(full_filename,data)
			else:
				ext = os.path.splitext(full_filename)[-1]
				if ext == '.exe':
					#print(full_filename)
					file_hash = hash(full_filename)
					#print(file_hash)
					data[full_filename] = file_hash
	except PermissionError:
		pass

	return data

def hash(file):

	f = open(file,'rb')
	data = f.read()
	hash = hashlib.sha256(data).hexdigest()
	#print(file)
	#print(hash)
	return hashh
	
def make_dir():
	try:
		if not(os.path.isdir("installer")):
			os.makedirs(os.path.join("installer"))
	except OSError as e:
		if e.errno != errno.EEXIST:
			print("실패실패!!")
			raise

def make_json(data):
	with open ('hash.json','w') as f:
		json.dump(data,f, ensure_ascii=False, indent="\t")


if __name__ =="__main__":
	download_file(path)
	find_hash("/mnt/c/Program Files/Wizvera",data)
	find_hash("/mnt/c/Program Files (x86)/Wizvera/",data)
	make_json(data)
