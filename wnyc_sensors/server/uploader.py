from md5 import md5
from threading import Thread 
from Queue import Queue 
import cPickle
from datetime import datetime
from time import mktime
import urllib
import os.path
from urllib2 import urlopen, HTTPError

class GoogleFormUploader(Thread):

    @classmethod
    def create_thread(cls):
        cls.que = Queue()
        thread = GoogleFormUploader()
        thread.last_visited = {}
        thread.daemon = True
        thread.name = "Uploader"
        thread.start()
        return thread

    def write(self, next):
        now = mktime(datetime.now().timetuple())
        open("/var/log/cicada.pickle", "a+").write(cPickle.dumps((now, next)))
        try:
            email = next['email']
            user = next['user']
            address = next['address']
            address_hash = md5(address).hexdigest()
            email_hash = md5(email).hexdigest()
            temperature = float(next['temp'])
        except KeyError:
            print "KeyError:", next
            return 
        except ValueError:
            print "Value Error:", next
            return 

        with open(os.path.join("/var/www/cicada/raw/upload", email_hash), "a+") as f:
            f.write("%02f %01f\n" % (now, temperature))
        if now - self.last_visited.get(address_hash, 0) < 24 * 60 * 60:
            print next
            return 

        try:
            req = ("https://docs.google.com/forms/d/1BW_nF1iRzOA9FeTGN9ulkpIDPVGZCc5ttllNH_kgftI/formResponse",
                   urllib.urlencode({"entry.528200367": user,
                                     "entry.1097490106": email,
                                     "entry.1853367646": address, 
                                     "entry.1163288024": temperature,
                                     "draftResponse": "[]",
                                     "pageHistory":"0",
                                     "submit":"Submit"}))
            print req
            print "Uploading: ", next, req
            urlopen(*req).read()
            self.last_visited[address_hash] = now
        except HTTPError:
            pass

    def run(self):
        print "Running"
        next = self.que.get(True)
        while next:
            self.write(next)
            next = self.que.get(True)
        print "Bye"
    
    def enque(self, item):
        self.que.put(item)

class GoogleForm(object):
    thread = None

    def write(self, data):
        if self.__class__.thread is None:
            self.__class__.thread = GoogleFormUploader.create_thread()
        self.thread.enque(data)
