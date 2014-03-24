# -*- coding: utf-8 -*-
"""This module contains the controller classes of the application."""

# symbols which are imported by "from TG_App.controllers import *"
__all__ = ['Root']

# standard library imports
import logging
import datetime

# third-party imports
from turbogears import controllers, expose, flash
# project specific imports
log = logging.getLogger('TG_App.controllers')
import scoped_model as model


class Root(controllers.RootController):
    """The root controller of the application."""

    @expose('TG_App.templates.welcome')
    def index(self):
        """Show the welcome page."""
        # log.debug("Happy TurboGears Controller Responding For Duty")
        flash(_(u"Your application is now running"))
        data = model.session.query(model.TB1).all()
        for entry in data:
            print entry,",",entry.tb2
        return dict(now=datetime.datetime.now())
