#!/usr/bin/env python
# ibus - The Input Bus
#
# Copyright(c) 2011 <buganini@gmail.com>
# Copyright(c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright(c) 2007-2010 Red Hat, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or(at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

# try Cython
# try:
#	 import pyximport
#	 pyximport.install(pyimport=True,build_in_temp=False)
# except:
#	 pass

CTRL_SPACE = chr(200)
CTRL_SHIFT = chr(201)
LEFT = chr(202)
RIGHT = chr(203)
SPACE = chr(204)
UP = chr(205)
DOWN = chr(206)
CTRL_LEFT = chr(207)
CTRL_RIGHT = chr(208)

import os
import sys
import ibus
import inputcontext
import gettext
import panel
import pynotify

from gettext import dgettext
_  = lambda a : dgettext("ibus", a)

class UIApplication:
	def __init__ (self):
		pynotify.init("ibus")
		self.__bus = ibus.Bus()
		self.__bus.connect("disconnected", sys.exit)
		self.__bus.connect("registry-changed", self.__registry_changed_cb)

		match_rule = "type='signal',\
					  sender='org.freedesktop.IBus',\
					  path='/org/freedesktop/IBus'"
		self.__bus.add_match(match_rule)

		self.__panel = panel.Panel(self.__bus)
                self.__context = inputcontext.InputContext(self.__bus)
		self.__context.set_capabilities(int('101000',2))
#		self.__bus.request_name(ibus.IBUS_SERVICE_PANEL, 0)
		self.__notify = pynotify.Notification("IBus", \
							_("Some input methods have been installed, removed or updated. " \
							"Please restart ibus input platform."), \
							"ibus")
		self.__notify.set_timeout(10 * 1000)
		self.__notify.add_action("restart", _("Restart Now"), self.__restart_cb, None)
		self.__notify.add_action("ignore", _("Later"), lambda *args: None, None)

	def __restart_cb(self, notify, action, data):
		if action == "restart":
			self.__bus.exit(True)

	def __registry_changed_cb(self, bus):
		self.__notify.show()

	def run(self):
		im_active=0
		im_n=0
		while 1:
			inp=os.read(sys.stdin.fileno(), 1)
			if(inp==UP):
				os.write(out,'UP')
			elif(inp==CTRL_SPACE or inp=='~'):
				if im_active:
					im_active=0
				else:
					im_active=1
				print "ibus status: %d" % im_active
			elif(inp==CTRL_SHIFT or inp==CTRL_RIGHT or inp=='>'):
				im_n+=1
				im_n%=len(self.__bus.list_active_engines())
				self.__context.focus_in()
				self.__context.set_engine(self.__bus.list_active_engines()[im_n])
				print self.__context.get_engine().name
			elif(inp==CTRL_LEFT or inp=='<'):
				im_n-=1
				im_n+=len(self.__bus.list_active_engines())
				im_n%=len(self.__bus.list_active_engines())
				self.__context.focus_in()
				self.__context.set_engine(self.__bus.list_active_engines()[im_n])
				print self.__context.get_engine().name
			else:
				if im_active:
					for c in inp:
						self.__context.process_key_event(ord(c), 0, 0)
				else:
					os.write(out, inp)

def main():
	UIApplication().run()

if __name__ == "__main__":
	localedir = os.getenv("IBUS_LOCALEDIR")
	gettext.bindtextdomain("ibus", localedir)
	gettext.bind_textdomain_codeset("ibus", "UTF-8")
	out=int(os.sys.argv[1])
	main()
