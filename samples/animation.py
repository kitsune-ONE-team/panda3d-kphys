import os

from panda3d.core import Filename
from kphys.core import BVHQ
import time


path = os.path.dirname(__file__)
anim_path = os.path.join(path, 'animation.bvhq')


for test in range(5):
    anim = BVHQ('animation.bvhq', Filename.from_os_specific(anim_path), True, True)
    print(anim)
    del anim
    time.sleep(1)

time.sleep(2)
print('OK')
