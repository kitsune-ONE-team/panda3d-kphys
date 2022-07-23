Physics extensions for Panda3D
==============================

Physics extensions for Panda3D originally made for the KITSUNETSUKI project game.


Features
--------

* Node-based actor system with IK and glTF loader
* Spring constraints from Bullet Physics


Building requirements
---------------------

* cmake
* bullet
* panda3d (full package with headers, not pip package)


Building python package
-----------------------

```
pip wheel --no-deps .
pip install *.whl
```


Building conda package
----------------------

```
git clone https://github.com/kitsune-ONE-team/panda3d-kphys.git
conda build .
conda install panda3d-kphys --use-local
```


Installing prebuild conda package
---------------------------------

```
conda config --add channels kitsune.one
conda install panda3d-kphys
```
