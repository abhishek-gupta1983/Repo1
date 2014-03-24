# -*- coding: utf-8 -*-
"""This module contains rules to map data model objects into JSON."""

# This file can be safely deleted from your project if do not use JSON
# controllers.

# A JSON-based API(view) for your app.
# Most rules would look like:
# @jsonify.when("isinstance(obj, YourClass)")
# def jsonify_yourclass(obj):
#     return [obj.val1, obj.val2]
# @jsonify can convert your objects to following types:
# lists, dicts, numbers and strings

from turbojson.jsonify import jsonify
