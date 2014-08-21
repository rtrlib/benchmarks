#!/usr/bin/python
from socket import *
import re
import xml
import xml.dom.minidom as dom
import string
import signal
from pprint import pprint

def parse(xml): 
  l = [] 
  try:
    tree = dom.parseString(xml)
  except xml.parsers.expat.ExpatError:
    print >> sys.stderr, xml
    return []

  d = {}
  for i in tree.firstChild.childNodes: 
    if i.nodeName == "ASCII_MSG": 
      schluessel = wert = None
      for elems in i.childNodes: 
        if elems.nodeName == "UPDATE": 
          for update in elems.childNodes:
            if(update.nodeName == "NLRI"):
              for nlri in update.childNodes:
                if(nlri.nodeName == "PREFIX"):
                  for prefix in nlri.childNodes:
                    if(prefix.nodeName == "ADDRESS"):
                      for txt in prefix.childNodes:
                        s = txt.data.split("/")
                        if not "prefix" in d:
                            d["prefix"] = []
                        p = {}
                        p["address"] =  s[0]
                        p["len"] = s[1]
                        d["prefix"].append(p)
            if(update.nodeName == "PATH_ATTRIBUTES"):
              for pattr in update.childNodes:
                if(pattr.nodeName == "ATTRIBUTE"):
                  for pattr in pattr.childNodes:
                    if(pattr.nodeName == "AS_PATH"):
                      for asPath in pattr.childNodes:
                        if(asPath.nodeName == "AS_SEG"):
                          length =  int(asPath.getAttribute("length"))
                          origin_as = asPath.childNodes[length-1]
                          d['as_path'] = asPath.childNodes
                          d["origin_as"] = str(origin_as.childNodes[0].data)
      break;
  if("prefix" in d):
    l.append(d)
  return l


cli = socket( AF_INET,SOCK_STREAM)
cli.connect(("livebgp.netsec.colostate.edu", 50001))
data =""
msg = ""
signal.signal(signal.SIGPIPE, signal.SIG_DFL)
signal.signal(signal.SIGINT, signal.SIG_IGN)

while True:
  data = cli.recv(1024) #14= </BGP_MESSAGE>
  if(re.search('</BGP_MESSAGE>', msg)):
    l = msg.split('</BGP_MESSAGE>', 1)
    bgp_update = l[0] + "</BGP_MESSAGE>"
    bgp_update = string.replace(bgp_update, "<xml>", "")
    d = parse(bgp_update)
    msg = ''.join(l[1:])
    for i in d:
      for j in i["prefix"]:
        path = []
        for k in i["as_path"]:
          path.append(str(k.childNodes[0].data))
        print(i["origin_as"]+ "\t"+ j["address"] + "\t" + j["len"] +"\t" + string.join(path, ","))
  msg += str(data)
