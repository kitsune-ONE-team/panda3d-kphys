from typing import Optional

from panda3d import bullet, core as p3d

from kphys.core import (
    SpringConstraint, Spring2Constraint,
    SPRING_DOF_RX, SPRING_DOF_RY, SPRING_DOF_RZ,
    SPRING_DOF_TX, SPRING_DOF_TY, SPRING_DOF_TZ,
    SPRING_RO_XZY,
)


class SpringMixin(object):
    def load_bullet_spring_chain(self, springid: int, vrm_chain: dict, gltf_data: dict):
        def create_body(bonenp, radius, length=0, mass=0):
            if length:
                shape = bullet.BulletCylinderShape(radius, length, bullet.YUp)
            else:
                shape = bullet.BulletSphereShape(radius)
            phynode = bullet.BulletRigidBodyNode(f'{bonenp.get_name()}.body')
            phynode.add_shape(shape)
            phynode.set_into_collide_mask(p3d.CollideMask.bit(self.springid))

            if mass:  # body is dynamic, sync bullet->panda3d
                phynode.set_mass(mass)
            else:  # body is kinematic (movable), sync panda3d->bullet
                phynode.set_kinematic(True)

            return phynode

        def create_constraint(body_a, body_b, distance_a, distance_b, hpr):
            # set spring attaching points relative to bodies center of mass
            transform_a = p3d.TransformState.make_pos_hpr(p3d.Point3(0, distance_a, 0), hpr)
            transform_b = p3d.TransformState.make_pos(p3d.Point3(0, -distance_b, 0))
            spring = Spring2Constraint(body_a, body_b, transform_a, transform_b, True)

            for dof in (SPRING_DOF_RY, SPRING_DOF_TX, SPRING_DOF_TY, SPRING_DOF_TZ):
                spring.set_limit(dof, 0, 0)

            for dof in (SPRING_DOF_RX, SPRING_DOF_RZ):
                spring.set_spring(dof, True)
                spring.set_limit(dof, -30, 30)
                spring.set_stiffness(dof, vrm_chain['stiffiness'])
                spring.set_damping(dof, vrm_chain['dragForce'])

                if hasattr(spring, 'set_bounce'):  # spring2 exclusive
                    spring.set_bounce(dof, 0)

            if hasattr(spring, 'set_rotation_order'):  # spring2 exclusive
                spring.set_rotation_order(SPRING_RO_XZY)

            body_b.set_python_tag('constraint', spring)

        def add_segment(bonenp, parent_phynp=None, parent_distance=0):
            if not bonenp.get_parent():
                 return []

            bone_length = bonenp.get_pos().length()
            body_radius = bone_length * 0.1
            body_padding = bone_length * 0.05

            if parent_phynp is None:  # create parent body
                if not bonenp.get_parent().get_parent():
                    return []

                body_length = body_padding * 2
                distance = body_length / 2 + body_padding

                # attach to grandparent bone
                phynode = create_body(
                    bonenp.get_parent().get_parent(), body_radius, body_length)
                phynp = bonenp.get_parent().get_parent().attach_new_node(phynode)
                # set the same position as parent bone
                phynp.set_pos(bonenp.get_parent().get_pos())
                # look at current bone
                phynp.look_at(bonenp)
                # move away from current bone
                phynp.set_y(phynp, -distance)

                return add_segment(bonenp, phynp, distance)

            else:  # create child body
                body_length = bone_length - body_padding * 2
                distance = body_length / 2 + body_padding

                # attach to parent body
                phynode = create_body(
                    bonenp.get_parent(), body_radius, body_length, mass=1)
                phynp = parent_phynp.attach_new_node(phynode)
                # move away from parent body
                phynp.set_y(phynp, parent_distance)
                # look at current bone
                phynp.look_at(bonenp)
                # move towards the current bone
                phynp.set_y(phynp, distance)

                # link with parent body
                if not phynode.is_kinematic():
                    create_constraint(
                        parent_phynp.node(), phynode,
                        parent_distance, distance,
                        phynp.get_hpr())

                bones_bodies = []
                bones_bodies.append((bonenp, phynp))
                for child_bonenp in bonenp.get_children():
                    bones_bodies += add_segment(child_bonenp, phynp, bone_length - distance)

                return bones_bodies

        bones_bodies = []
        for nodeid in vrm_chain.get('bones'):
            for bonenp in self.node_paths[nodeid].get_children():
                bones_bodies += add_segment(bonenp)
                self.springid += 1
                if self.springid >= 32:
                    self.springid = 0

        for np, phynp in bones_bodies:
            np.reparent_to(phynp)
            np.set_pos(0, 0, 0)
            np.set_hpr(0, 0, 0)
