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
			if(kb[0]=="'" and kb[2]=="'"):
				self.keysym_to_keycode_map[kb[1]]=(kcode,0)
			if(ks[0]=="'" and ks[2]=="'"):
				self.keysym_to_keycode_map[ks[1]]=(kcode,0)
			if(kc[0]=="'" and kc[2]=="'"):
				self.keysym_to_keycode_map[kc[1]]=(kcode,0)
			if(kcs[0]=="'" and kcs[2]=="'"):
				self.keysym_to_keycode_map[kcs[1]]=(kcode,0)
			if(ka[0]=="'" and ka[2]=="'"):
				self.keysym_to_keycode_map[ka[1]]=(kcode,0)
			if(kas[0]=="'" and kas[2]=="'"):
				self.keysym_to_keycode_map[kas[1]]=(kcode,0)
			if(kac[0]=="'" and kac[2]=="'"):
				self.keysym_to_keycode_map[kac[1]]=(kcode,0)
			if(kacs[0]=="'" and kacs[2]=="'"):
				self.keysym_to_keycode_map[kacs[1]]=(kcode,0)
		f.close()

	def keysym_to_keycode(self, sym):
		return self.keysym_to_keycode_map[sym]

