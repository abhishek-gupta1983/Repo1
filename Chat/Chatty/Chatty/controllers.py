# -*- coding: utf-8 -*-
"""This module contains the controller classes of the application."""

# symbols which are imported by "from Chatty.controllers import *"
__all__ = ['Root']

# standard library imports
# import logging
import datetime

# third-party imports
from turbogears import controllers, expose, flash

# project specific imports
# from Chatty import model
# from Chatty import jsonify


# log = logging.getLogger('Chatty.controllers')


class Root(controllers.RootController):
    """The root controller of the application."""

    @expose('Chatty.templates.welcome')
    def index(self):
        """Show the welcome page."""
        # log.debug("Happy TurboGears Controller Responding For Duty")
        flash(_(u"Your application is now running"))
        return dict(now=datetime.datetime.now())
