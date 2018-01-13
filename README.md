RenderDoc Autotesting
==============

This is a suite of tiny programs and control logic for auto-testing RenderDoc on small features and bugs, to make sure nothing has regressed.

Tests are added to stress specific features, entry points, pieces of functionality etc. Some are written specifically, some are written after the fact to verify a bugfix made to renderdoc. They test both capture and replay (ie. can we correctly replay the results of an application) as well as analysis functionality (can we debug a shader correctly, display the correct pipeline state, etc).

The main repository is [renderdoc](https://github.com/baldurk/renderdoc), go there for more information!

Building
--------------

I assume VS 2015 and latest Windows SDK.

Building on linux/android to come soon.

License
--------------

RenderDoc is released under the MIT license, see [LICENSE.md](LICENSE.md) for full text. This repository is under the same license.

The tests use [GLAD](https://github.com/Dav1dde/glad) for extension loading, which is MIT licensed.

Also [GLFW](https://github.com/glfw/glfw) for window creation and handling, which is Zlib licensed.
