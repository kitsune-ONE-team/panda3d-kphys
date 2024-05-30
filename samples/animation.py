import os

from panda3d.core import Filename
from kphys.core import BVHQ


path = os.path.dirname(__file__)
anim_path = os.path.join(path, 'animation.bvhq')


for test in range(5):
    anim = BVHQ('animation.bvhq', Filename.from_os_specific(anim_path), True, True)
    print(anim)
