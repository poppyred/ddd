#!/usr/bin/python


import datetime
import pymongo

now = datetime.datetime.now()

today=datetime.datetime.now()  
oneday=datetime.timedelta(hours=1)  
date=today-oneday
date = date.strftime('%Y-%m-%d-%H')

elmts = date.split('-')
year = elmts[0]
month = elmts[1]
day = elmts[2]
hour = elmts[3]
#hour = '00'

dir = '/opt/dnsproxy/log/reqcnt/' + date[:10] + '/' + hour

file_object = open(dir)
try:
     text = file_object.read( )
	 
finally:
     file_object.close( )
	 
	
#print text
text = text.split('\n')
text.remove(text[len(text)-1])
#print text

d = {}

for element in text:
	#print element
	element = element.split('\t')
	#print element
	#print element[0]
	
	
	domain = element[1]
	view = element[2]
	reqcnt = int(element[3])
	
	#print(domain)
	#print(view)
	#print(reqcnt)
	
	#print d.has_key(domain)
	
	if domain in d:
		if view in d[domain]:
			d[domain][view] += reqcnt
			continue

		d[domain][view] = 0
		d[domain][view] += reqcnt
		continue	
		
	d[domain] = {}
	d[domain][view] = 0
	d[domain][view] += reqcnt

'''
for element in text:
	#print element
	element = element.split('\t')
	#print element
	#print element[0]
	
	domain = element[1]
	view = element[2]
	reqcnt = int(element[3])
	
	
	#print d.has_key(domain)	
	if domain in e:
	
		e[domain] += reqcnt
		continue	

	e[domain] = 0
	e[domain] += reqcnt
  	
print e
'''

#print d

conn = pymongo.Connection('121.201.12.60',22235)
db = conn['dnspro']


#for item in db.reqcnt.find():
   # print item

for key in d:
	for view in d[key]:
		#print key, view, d[key][view]
		
		data = {'year':year,'month':month,'day':day,'hour':hour,'domain':key,'view':view,'req':d[key][view]}
		print data
		db.reqcnt.insert(data);
		
		name = year + '-' + month
		collection = db[name]
		collection.insert(data);
		
		
		
	
	