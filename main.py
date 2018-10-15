# -*- coding: utf-8 -*-

import json
from collections import OrderedDict

f = open("MouseTrap.log", encoding="ISO-8859-1")
g = open("result.json",'w', encoding="utf-8")

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

        print(info[1].encode('utf-8'))      # guid
        print(info[2].encode('utf-8'))      # name
        print(info[3].encode('utf-8'))      #version
        print(info[4].encode('utf-8'))      #uninstaller

        json.dump(data, g, ensure_ascii=False, indent="\t")


