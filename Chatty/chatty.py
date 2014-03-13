#!/usr/bin/env python

import logging
import tornado.auth
import tornado.escape
import tornado.ioloop
import tornado.web
import os.path
import uuid

from tornado import gen
from tornado.options import define, options, parse_command_line

from multiprocessing import Queue

__all__ = ['MainHandler','UserJoining','GetJoinedUsers','MessageNewHandler','MessageUpdateHandler']

class User():
    def __init__(self, userName):
        self.name = userName
        self.messageCache = []
        self.waiters = set()
        self.cache_size = 200

    def wait_for_messages(self, callback, cursor=None):
        if cursor:
            new_count = 0
            for msg in reversed(self.messageCache):
                if msg["id"] == cursor:
                    break
                new_count += 1
            if new_count:
                callback(self.messageCache[-new_count:])
                return
        self.waiters.add(callback)
        print "number of waiters:", len(self.waiters)
    
    def postNewMessage(self, message):
        print "Adding message:", message
        for callback in self.waiters:
            try:
                callback(message)
            except:
                logging.error("Error in waiter callback", exc_info=True)
        self.waiters = set()
        self.messageCache.extend(message)
        if len(self.messageCache) > self.cache_size:
            self.messageCache = self.messageCache[-self.cache_size:]
            
        #self.messageCache.append(message)

class ConnectedUsers():
    def __init__(self):
        self.users = set()
        
    def addUser(self, userName):
        user = User(userName)
        self.users.add(user)
    
    def getUser(self, userName):
        for user in self.users:
            if user.name == userName:
                return user
        return None
    
connectedUsers = ConnectedUsers()

class BaseHandler(tornado.web.RequestHandler):
    def get_current_user(self):
        user_json = self.get_secure_cookie("chat_user")
        if not user_json: return None
        return tornado.escape.json_decode(user_json)

class MainHandler(BaseHandler):
    def get(self):
        # TODO: returning those messages which were sent in offline mode.
        self.render("index.html", messages=[])
        
class UserJoining(BaseHandler):
    def get(self):
        userName = self.get_argument("userName")
        print userName," is joining chat...."
        connectedUsers.addUser(userName)
        self.set_secure_cookie("chat_user",
                               tornado.escape.json_encode(userName))

class GetJoinedUsers(BaseHandler):
    def get(self):
        userName = self.get_current_user()
        if not userName:
            return None
        print "Getting signed in users for:", userName
        signedInUsers = [user.name for user in connectedUsers.users if user.name!=userName]
        self.finish(dict(signedInUsers=signedInUsers))

class MessageNewHandler(BaseHandler):
    def post(self):
        userName = self.get_current_user()
        if not userName:
            print "User has been signed out."
            return None
        
        message = {
            "id": str(uuid.uuid4()),
            "from": userName,
            "to":   self.get_argument("to"),
            "body": self.get_argument("body"),
        }
        
        message["html"] = tornado.escape.to_basestring(
            self.render_string("message.html", message=message))
        if self.get_argument("next", None):
            self.redirect(self.get_argument("next"))
        else:
            self.write(message)
        
        user = connectedUsers.getUser(message["to"])
        user.postNewMessage([message])

class MessageUpdateHandler(BaseHandler):
    @tornado.web.asynchronous
    def post(self):
        userName = self.get_current_user()
        print "Fetching message updates for:",userName
        if not userName:
            return None
        
        user = connectedUsers.getUser(userName)
        if not user:
            return None
        cursor = self.get_argument("cursor", None)
        user.wait_for_messages(self.on_new_messages,
                               cursor=cursor)
        
    def on_new_messages(self, messages):
        # Closed client connection
        print "Sending:",messages
        if self.request.connection.stream.closed():
            return
        self.finish(dict(messages=messages))

    def on_connection_close(self):
        print "Connection closed"

    
    
class UserLeaving(BaseHandler):
    def get(self, userName):
        pass
    

    
