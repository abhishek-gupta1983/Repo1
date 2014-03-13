#!/usr/bin/env python
import logging
import tornado.auth
import tornado.escape
import tornado.ioloop
import tornado.web
import os.path
import uuid
import signal

from tornado import gen
from tornado.options import define, options, parse_command_line

from chatty import *

define("port", default=8888, help="run on the given port", type=int)

is_closing=False

def signal_handler(signum, frame):
    global is_closing
    logging.info('exiting...')
    is_closing = True

def try_exit(): 
    global is_closing
    if is_closing:
        # clean up here
        tornado.ioloop.IOLoop.instance().stop()
        logging.info('exit success')
        
def main():
    parse_command_line()
    app = tornado.web.Application(
        [
            (r"/", MainHandler),
            (r"/chat/user/joining", UserJoining),
            (r"/chat/user/friends", GetJoinedUsers),
            (r"/chat/message/new", MessageNewHandler),
            (r"/chat/message/updates", MessageUpdateHandler),
            ],
        #login_url="/auth/login",
        cookie_secret="__TODO:_GENERATE_YOUR_OWN_RANDOM_VALUE_HERE__",
        template_path=os.path.join(os.path.dirname(__file__), "templates"),
        static_path=os.path.join(os.path.dirname(__file__), "static"),
        xsrf_cookies=True,
        )
    app.listen(options.port)
    signal.signal(signal.SIGINT, signal_handler)
    tornado.ioloop.PeriodicCallback(try_exit, 100).start()
    tornado.ioloop.IOLoop.instance().start()


if __name__ == "__main__":
    main()