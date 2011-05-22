#
# Copyright (c) 2011 Kuan-Chung Chiu <buganini@gmail.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
# OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

import os
import re
from ibus.modifier import *

wrap={'\r':'cr', '\n':'nl', '\x7f':'bs'}
for c in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ 1234567890-=!@#$%^&*()_+;',./`\\:\"<>?~|":
	wrap[c]="'%s'"%c

class keymap():
	def __init__(self, keymap):
		self.keymap=os.path.join('/usr/share/syscons/keymaps',keymap+'.kbd')
		self.keysym_to_keycode_map={}
		f=open(self.keymap,'r')
		for l in f.readlines():
			l=l.strip()
			if l=='' or l[0]=='#':
				continue
			(kcode, kb, ks, kc, kcs, ka, kas, kac, kacs, kl) = re.findall(r"('.'|\w+)", l)
			kcode=int(kcode)
			self.keysym_to_keycode_map[kacs]=(kcode, ALT_MASK|CONTROL_MASK|SHIFT_MASK)
			self.keysym_to_keycode_map[kac]=(kcode, ALT_MASK|CONTROL_MASK)
			self.keysym_to_keycode_map[kas]=(kcode, ALT_MASK|SHIFT_MASK)
			self.keysym_to_keycode_map[ka]=(kcode, ALT_MASK)
			self.keysym_to_keycode_map[kcs]=(kcode, CONTROL_MASK|SHIFT_MASK)
			self.keysym_to_keycode_map[kc]=(kcode, CONTROL_MASK)
			self.keysym_to_keycode_map[ks]=(kcode, SHIFT_MASK)
			self.keysym_to_keycode_map[kb]=(kcode, 0)
		f.close()

	def keysym_to_keycode(self, sym):
		return self.keysym_to_keycode_map[wrap[sym]]

