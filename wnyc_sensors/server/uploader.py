from md5 import md5
from threading import Thread 
from Queue import Queue 
from datetime import datetime
from time import mktime
import urllib

class GoogleFormUploader(object):

    @classmethod
    def create_thread(cls):
        cls.que = Queue()
        thread.daemon = True
        thread.name = "Uploader"
        thread = Thread(cls)
        thread.start()
        return thread

    def write(self, next):
        now = mtime(datetime.now().timetuple())
        open("/var/log/cicada.pickle", "a+").write(cPickle.dumps((now, next)))
        try:
            email = next['email']
            address = next['address']
            address_hash = md5(address).hexdigest()
            email_hash = md5(email).hexdigest()
            temperature = float(next['temp'])
        except KeyError:
            return 
        except ValueError:
            return 

        with open("/var/wwwdata/cicada/" + email_hash, "a+") as f:
            f.write("%02f %01d" % (now, temperature))

        if now - self.last_visited.get(address_hash, 0) < 24 * 60 * 60:
            return 
        
        try:
            urlopen("https://docs.google.com/forms/d/1BW_nF1iRzOA9FeTGN9ulkpIDPVGZCc5ttllNH_kgftI/formResponse",
                    urllib.urlencode({"entry.528200367": name,
                                      "entry.1097490106": email,
                                      "entry.1853367646": address, 
                                      "entry.1163288024": temp,
                                      "draftResponse": "[]",
                                      "pageHistory":"0",
                                      "submit":"Submit"})).read()
            self.last_visited[hash] = now
        except HttpError:
            pass

    def run(self):
        self.last_visited = {}
        next = self.que.get(True)
        while next:
            self.write(next)
            next = self.que.get(True)
    
    def enque(self, item):
        self.que.put(item)

class GoogleForm(object):
    thread = None

    def write(self, data):
        if thread is None:
            thread = GoogleFormUploader.create_thread()
        self.thread.enque(data)
