import os
import sys

from direct.showbase.ShowBase import ShowBase

from panda3d.core import load_prc_file_data, NodePath, Shader

from kphys.core import EffectorNode
from kphys.loader import load_actor

from common import make_empty


class ActorSample(ShowBase):
    def __init__(self):
        load_prc_file_data('', '''
            framebuffer-alpha f
            win-size 1280 720
        ''')

        ShowBase.__init__(self)

        self.cam.set_y(-4)
        self.cam.set_z(0.7)

        filepath = os.path.join(os.path.dirname(__file__), 'yuki', 'scene.gltf')
        scene = load_actor(filepath)
        scene.ls()
        self._actor = scene.find('**/+ArmatureNode')
        self._actor.reparent_to(self.render)
        self._actor.node().update_shader_inputs()

        for bone in self._actor.find_all_matches('**/+BoneNode'):
            make_empty(parent=bone)

        bone = self._actor.find('**/mixamorig:Head_06')
        self._effector_t = NodePath(EffectorNode('Effector Head', chain_length=4, priority=0))
        self._effector_t.reparent_to(bone)
        make_empty(parent=self._effector_t)

        bone = self._actor.find('**/mixamorig:LeftHand_011')
        self._effector_l = NodePath(EffectorNode('Effector Hand_L', chain_length=2, priority=1))
        self._effector_l.reparent_to(bone)
        make_empty(parent=self._effector_l)

        bone = self._actor.find('**/mixamorig:RightHand_026')
        self._effector_r = NodePath(EffectorNode('Effector Hand_R', chain_length=2, priority=2))
        self._effector_r.reparent_to(bone)
        make_empty(parent=self._effector_r)

        self._actor.node().rebuild_bind_pose()
        self._actor.node().rebuild_ik()

        shader_parts = {}
        for stype in ('fragment', 'vertex'):
            ftype = stype[:4]
            filepath = os.path.join(os.path.dirname(__file__), f'actor.{ftype}.glsl')
            with open(filepath, 'r') as f:
                shader_parts[stype] = f.read()
        shader = Shader.make(Shader.SL_GLSL, **shader_parts)
        for geom in self._actor.find_all_matches('**/+GeomNode'):
            if geom.get_name() != 'lineNode':  # don't apply skinning to axis lines
                geom.set_shader(shader)

        self.accept('escape', sys.exit)
        self.accept('f3', self.toggleWireframe)
        self.accept('l', self._actor.ls)
        self.accept('r', self._actor.node().reset)
        self.task_mgr.add(self._update, '_update')

        for k in (tuple('wasduhjk') + ('arrow_left', 'arrow_right', 'arrow_up', 'arrow_down')):
            self.accept(k, self._set_key, [k, True])
            self.accept(f'{k}-up', self._set_key, [k, False])

        self._keymap = {}

    def _set_key(self, key, value):
        self._keymap[key] = value

    def _update(self, task):
        dt = self.clock.dt

        if self._keymap.get('arrow_left'):
            x = self._effector_t.get_x(self.render)
            self._effector_t.set_x(self.render, x - 1 * dt)
        if self._keymap.get('arrow_right'):
            x = self._effector_t.get_x(self.render)
            self._effector_t.set_x(self.render, x + 1 * dt)
        if self._keymap.get('arrow_up'):
            z = self._effector_t.get_z(self.render)
            self._effector_t.set_z(self.render, z + 1 * dt)
        if self._keymap.get('arrow_down'):
            z = self._effector_t.get_z(self.render)
            self._effector_t.set_z(self.render, z - 1 * dt)

        if self._keymap.get('a'):
            x = self._effector_r.get_x(self.render)
            self._effector_r.set_x(self.render, x - 1 * dt)
        if self._keymap.get('d'):
            x = self._effector_r.get_x(self.render)
            self._effector_r.set_x(self.render, x + 1 * dt)
        if self._keymap.get('w'):
            z = self._effector_r.get_z(self.render)
            self._effector_r.set_z(self.render, z + 1 * dt)
        if self._keymap.get('s'):
            z = self._effector_r.get_z(self.render)
            self._effector_r.set_z(self.render, z - 1 * dt)

        if self._keymap.get('h'):
            x = self._effector_l.get_x(self.render)
            self._effector_l.set_x(self.render, x - 1 * dt)
        if self._keymap.get('k'):
            x = self._effector_l.get_x(self.render)
            self._effector_l.set_x(self.render, x + 1 * dt)
        if self._keymap.get('u'):
            z = self._effector_l.get_z(self.render)
            self._effector_l.set_z(self.render, z + 1 * dt)
        if self._keymap.get('j'):
            z = self._effector_l.get_z(self.render)
            self._effector_l.set_z(self.render, z - 1 * dt)

        self._actor.node().reset()
        self._actor.node().update_ik()
        self._actor.node().update_shader_inputs()

        return task.again


sample = ActorSample()
sample.run()
