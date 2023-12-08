#!/usr/bin/env python3
import sys

from direct.showbase.ShowBase import ShowBase

from panda3d.bullet import BulletBoxShape, BulletGhostNode
from panda3d.core import load_prc_file_data, LVecBase3, NodePath

from kphys.core import HitboxNode


class HitboxSample(ShowBase):
    def __init__(self):
        load_prc_file_data('', '''
            framebuffer-alpha f
            win-size 1280 720
        ''')

        ShowBase.__init__(self)

        ghost = BulletGhostNode()
        ghost.add_shape(BulletBoxShape(LVecBase3(1, 2, 3)))
        self._hitbox = NodePath(HitboxNode('hitbox', ghost))
        self._hitbox.reparent_to(self.render)
        self._hitbox.node().show_debug_node()

        self.cam.set_y(-20)

        self.accept('f3', self.toggleWireframe)
        self.accept('escape', sys.exit)
        self.accept('q', sys.exit)


sample = HitboxSample()
sample.run()
