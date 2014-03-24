# -*- coding: utf-8 -*-
"""Unit test cases for testing the application's controller methods

See http://docs.turbogears.org/1.5/Testing#testing-your-controller for more
information.

"""
import unittest
import datetime
from turbogears import testutil
from TG_App.controllers import Root


class TestPages(testutil.TGTest):

    root = Root

    def test_method(self):
        """The index method should return a datetime.datetime called 'now'"""
        response = self.app.get('/')
        assert isinstance(response.raw['now'], datetime.datetime)

    def test_index_title(self):
        """"The index page should have the right title."""
        response = self.app.get('/')
        assert "<title>Welcome to TurboGears</title>" in response.body

