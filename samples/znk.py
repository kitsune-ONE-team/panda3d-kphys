#!/usr/bin/env python3
import os
import sys

from direct.showbase.ShowBase import ShowBase
from direct.gui.OnscreenText import OnscreenText

from panda3d.core import load_prc_file_data, NodePath, Shader, Vec3, TextNode
from panda3d.bullet import BulletDebugNode, BulletWorld

from kphys.core import EffectorNode, IK_ENGINE_IK, IK_ENGINE_CCDIK
from kphys.loader import load_model

from common import make_empty


class ActorSample(ShowBase):
    def __init__(self):
        load_prc_file_data('', '''
            bullet-filter-algorithm groups-mask
            framebuffer-alpha f
            show-frame-rate-meter t
            win-size 1280 720
        ''')

        ShowBase.__init__(self)

        self._bullet_debug = NodePath(BulletDebugNode())
        self._bullet_debug.reparent_to(self.render)
        self._bullet_debug.show()

        self._bullet_world = BulletWorld()
        self._bullet_world.set_gravity(Vec3(0, 0, -9.8))
        self._bullet_world.set_debug_node(self._bullet_debug.node())
        for i in range(32):
            for j in range(32):
                self._bullet_world.set_group_collision_flag(i, j, i == j)

        self.cam.set_y(-4)
        self.cam.set_z(0.7)
        self.cam.node().get_lens().set_near_far(0.01, 100.0)

        self._actors = []
        self._effectors = {
            'top': [],
            'left': [],
            'right': [],
        }
        self._texts = []
        for i in range(1):
            filepath_a = os.path.join(os.path.dirname(__file__), 'Zonko_VRM_221128_ps.vrm')
            filepath_b = os.path.join(os.path.dirname(__file__), 'znk.vrm')
            node = None
            if os.path.exists(filepath_a):
                node = load_model(filepath_a)
            elif os.path.exists(filepath_b):
                node = load_model(filepath_b)
            else:
                print('Avatar model not found.')
                print('You can download the original version from:')
                print('https://github.com/murasaqi/ZONKO_3D_OPEN_SOURCE_PROJECT_MurasaqiExample/blob/main/Unity_ZONKO_3D_OPEN_SOURCE_PROJECT/Assets/Zone_LiveDemo/Models/znk/znk_221107/vrm/znk.vrm')
                print('or improved version (with spring bones) from:')
                print('https://booth.pm/ja/items/4469670')
                sys.exit(1)

            scene = NodePath(node)
            actor = scene.find('**/+ArmatureNode')
            actor.reparent_to(self.render)
            actor.set_x(i - 0.5)
            actor.set_z(-0.2)
            actor.set_h(180)
            actor.node().update_shader_inputs()

            for np in actor.find_all_matches('**/+BulletRigidBodyNode'):
                self._bullet_world.attach_rigid_body(np.node())
                if np.get_python_tag('constraint'):
                    self._bullet_world.attach_constraint(np.get_python_tag('constraint'))

            actor.find('**/JacketB_A_geo').hide()
            actor.find('**/JacketBackB_geo').hide()
            actor.find('**/jaketB_top_geo').hide()
            actor.find('**/zipperB_geo').hide()

            bone = actor.find('**/Head')
            effector = NodePath(EffectorNode('Effector Head', chain_length=4, priority=0))
            effector.reparent_to(bone)
            self._effectors['top'].append(effector)

            bone = actor.find('**/LeftHand')
            effector = NodePath(EffectorNode('Effector Hand_L', chain_length=2, priority=1))
            effector.reparent_to(bone)
            self._effectors['left'].append(effector)

            bone = actor.find('**/RightHand')
            effector = NodePath(EffectorNode('Effector Hand_R', chain_length=2, priority=1))
            effector.reparent_to(bone)
            self._effectors['right'].append(effector)

            for bone in actor.find_all_matches('**/+BoneNode'):
                if bone.get_children():
                    bone_length = bone.get_children()[0].get_pos().length()
                    make_empty(parent=bone, size=bone_length * 0.25)
                else:
                    make_empty(parent=bone)
            for bone in actor.find_all_matches('**/+EffectorNode'):
                make_empty(parent=bone)

            actor.node().rebuild_bind_pose()
            actor.node().rebuild_ik(IK_ENGINE_CCDIK if i == 0 else IK_ENGINE_IK)
            text = OnscreenText(
                text='CCDIK' if i == 0 else 'FABRIK',
                pos=(i - 0.5, 0.75),
                align=TextNode.ACenter,
                parent=self.render2d)
            self._texts.append(text)
            self._actors.append(actor)

            shader_parts = {}
            for stype in ('fragment', 'vertex'):
                ftype = stype[:4]
                filepath = os.path.join(os.path.dirname(__file__), f'yuki.{ftype}.glsl')
                with open(filepath, 'r') as f:
                    shader_parts[stype] = f.read()

            shader = Shader.make(Shader.SL_GLSL, **shader_parts)
            for geom in actor.find_all_matches('**/+GeomNode'):
                if geom.get_name() != 'lineNode':  # don't apply skinning to axis lines
                    geom.set_shader(shader)
                    # geom.hide()

        self.accept('f3', self.toggleWireframe)
        self.accept('escape', sys.exit)
        self.accept('q', sys.exit)
        self.accept('l', self._actors_ls)
        self.accept('r', self._actors_reset)
        self.task_mgr.add(self._update, '_update')

        for k in tuple('wasduhjk') + ('arrow_up', 'arrow_left', 'arrow_down', 'arrow_right'):
            self.accept(k, self._set_key, [k, True])
            self.accept(f'{k}-up', self._set_key, [k, False])

        self._keymap = {}

    def _set_key(self, key, value):
        self._keymap[key] = value

    def _actors_ls(self):
        for actor in self._actors:
            actor.ls()

    def _actors_reset(self):
        for actor in self._actors:
            actor.node().reset()

    def _move_effectors(self, etype, direction, delta):
        for effector in self._effectors[etype]:
            getter = getattr(effector, f'get_{direction}')
            setter = getattr(effector, f'set_{direction}')
            value = getter(self.render)
            setter(self.render, value + delta)

    def _update(self, task):
        dt = self.clock.dt

        effectors = ('left', 'right', 'top')
        args = (
            ('z', dt),
            ('x', -dt),
            ('z', -dt),
            ('x', dt),
        )
        for i, k in enumerate(tuple('wasduhjk') + ('arrow_up', 'arrow_left', 'arrow_down', 'arrow_right')):
            if self._keymap.get(k):
                self._move_effectors(effectors[i // 4], *args[i % 4])

        for actor in self._actors:
            actor.node().update_shader_inputs()
            # actor.node().reset()
            actor.node().update_wiggle_bones(self.render, dt)
            actor.node().update_ik()

        self._bullet_world.do_physics(dt, 60, 1 / 60)

        return task.again


sample = ActorSample()
sample.run()
