import json
from collections import OrderedDict

f = open("MouseTrap.log", encoding="ISO-8859-1")
g = open("result.json",'w')

data = OrderedDict()

while True:
    line = f.readline()

    if not line:
        break;
    else:
        info = line.split(",")
        data["guid"] = info[1]
        data["name"] = info[2]
        data["version"] = info[3]
        data["uninstaller"] = info[4]

        print(info[1])      # guid
        print(info[2])      # name
        print(info[3])      #version
        print(info[4])      #uninstaller

        json.dump(data, g, ensure_ascii=False, indent="\t")


