# Copyright (c) 2011 and onwards, buganini@gmail.com
# Copyright (c) 2010 and onwards, S. Irie

# Author: S. Irie
# Maintainer: S. Irie
# Version: 0.2.1

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import ibus
import re

########################################################################
# Miscellaneous functions
########################################################################

def lisp_boolean(boolean):
	return "t" if boolean else "nil"

escape_regexp = re.compile(ur'(["\\])')

def escape_string(string):
	return escape_regexp.sub(ur'\\\1', string)

########################################################################
# Input Context
########################################################################

class InputContext(ibus.InputContext):

	def __init__(self, bus):
		self.__bus = bus
		self.__path = bus.create_input_context("InputContext")
		super(InputContext, self).__init__(bus, self.__path, True)

		self.id_no = 0
		self.preediting = False
		self.lookup_table = None

		self.connect('commit-text', commit_text_cb)
		self.connect('update-preedit-text', update_preedit_text_cb)
		self.connect('show-preedit-text', show_preedit_text_cb)
		self.connect('hide-preedit-text', hide_preedit_text_cb)
		self.connect('update-auxiliary-text', update_auxiliary_text_cb)
		self.connect('show-auxiliary-text', show_auxiliary_text_cb)
		self.connect('hide-auxiliary-text', hide_auxiliary_text_cb)
		self.connect('update-lookup-table', update_lookup_table_cb)
		self.connect('show-lookup-table', show_lookup_table_cb)
		self.connect('hide-lookup-table', hide_lookup_table_cb)
		self.connect('page-up-lookup-table', page_up_lookup_table_cb)
		self.connect('page-down-lookup-table', page_down_lookup_table_cb)
		self.connect('cursor-up-lookup-table', cursor_up_lookup_table_cb)
		self.connect('cursor-down-lookup-table', cursor_down_lookup_table_cb)
		self.connect('enabled', enabled_cb)
		self.connect('disabled', disabled_cb)
		try:
			self.connect('forward-key-event', forward_key_event_cb)
		except TypeError:
			pass
		try:
			self.connect('delete-surrounding-text', delete_surrounding_text_cb)
		except TypeError:
			pass

########################################################################
# Callbacks
########################################################################

def commit_text_cb(ic, text):
	print '(ibus-commit-text-cb %d "%s")'% \
		(ic.id_no, escape_string(text.text).encode("utf-8"))

def update_preedit_text_cb(ic, text, cursor_pos, visible):
	preediting = len(text.text) > 0
	if preediting or ic.preediting:
		attrs = ['%s %d %d %d'%
				 (["nil", "'underline", "'foreground", "'background"][attr.type],
				  attr.value & 0xffffff, attr.start_index, attr.end_index)
				 for attr in text.attributes]
		print '(ibus-update-preedit-text-cb %d "%s" %d %s %s)'% \
			(ic.id_no, escape_string(text.text).encode("utf-8"),
			 cursor_pos, lisp_boolean(visible), ' '.join(attrs))
	ic.preediting = preediting

def show_preedit_text_cb(ic):
	print '(ibus-show-preedit-text-cb %d)'%(ic.id_no)

def hide_preedit_text_cb(ic):
	print '(ibus-hide-preedit-text-cb %d)'%(ic.id_no)

def update_auxiliary_text_cb(ic, text, visible):
	print '(ibus-update-auxiliary-text-cb %d "%s" %s)'% \
		(ic.id_no, escape_string(text.text).encode("utf-8"),
		 lisp_boolean(visible))

def show_auxiliary_text_cb(ic):
	print '(ibus-show-auxiliary-text-cb %d)'%(ic.id_no)

def hide_auxiliary_text_cb(ic):
	print '(ibus-hide-auxiliary-text-cb %d)'%(ic.id_no)

def update_lookup_table_cb(ic, lookup_table, visible):
	ic.lookup_table = lookup_table
	if visible:
		self.__show_lookup_table_cb(ic)
	else:
		self.__hide_lookup_table_cb(ic)

def show_lookup_table_cb(ic):
	print "(ibus-show-lookup-table-cb %d '(%s) %s)"% \
		(ic.id_no, escape_string(
			" ".join(map(lambda item : '"%s"'%item.text,
						 ic.lookup_table.get_candidates_in_current_page())
					 )).encode("utf-8"),
		 ic.lookup_table.get_cursor_pos_in_current_page())

def hide_lookup_table_cb(ic):
	print '(ibus-hide-lookup-table-cb %d)'%(ic.id_no)

def page_up_lookup_table_cb(ic):
	print '(ibus-log "page up lookup table")'

def page_down_lookup_table_cb(ic):
	print '(ibus-log "page down lookup table")'

def cursor_up_lookup_table_cb(ic):
	print '(ibus-log "cursor up lookup table")'

def cursor_down_lookup_table_cb(ic):
	print '(ibus-log "cursor down lookup table")'

def enabled_cb(ic):
	print '(ibus-status-changed-cb %d "%s")'%(ic.id_no, ic.get_engine().name)

def disabled_cb(ic):
	print '(ibus-status-changed-cb %d nil)'%ic.id_no

def forward_key_event_cb(ic, keyval, keycode, modifiers):
	print '(ibus-forward-key-event-cb %d %d %d %s)'% \
		(ic.id_no, keyval, modifiers & ~modifier.RELEASE_MASK,
		 lisp_boolean(modifiers & modifier.RELEASE_MASK == 0))

def delete_surrounding_text_cb(ic, offset, n_chars):
	print '(ibus-delete-surrounding-text-cb %d %d %d)'% \
		(ic.id_no, offset, n_chars)
