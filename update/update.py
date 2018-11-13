import os
import requests
import json
import hashlib
from pprint import pprint

#print(os.getcwd())

pwd = os.getcwd()
path = pwd +'/installer/'


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
	    get_file_hash(file)


def get_file_hash(file):

		f = open(file,'rb')
		data = f.read()
		hash = hashlib.sha1(data).hexdigest()
		print(file)
		print(hash)
	
def make_dir():
	try:
		if not(os.path.isdir("installer")):
			os.makedirs(os.path.join("installer"))
	except OSError as e:
		if e.errno != errno.EEXIST:
			print("실패실패!!")
			raise

#def make_json():


if __name__ =="__main__":
	download_file(path)