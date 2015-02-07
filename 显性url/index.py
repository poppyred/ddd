#!/usr/bin/env python 

import web        

urls = (
        "/favicon\.ico", "favicon",
         '/(.*)', 'hello'
         )
app = web.application(urls, globals())

class hello:     
    def GET(self, name):
        print 'name', name
        usr_data = web.input()
        web_data = web.data()
        print 'web_data', web_data
        print 'usr_data', usr_data        
        return usr_data.name


class favicon:
    def GET(self):
        return 'ok'      
    


if __name__ == "__main__":
    app.run()